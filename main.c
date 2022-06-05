
/**

Project - Publisher for Mercure Rocks Server (SSE ) using C Language
with LibCurl, b64 (Apple) and GNU hmac sha256
Only for GCC Gnu/Linux

Copyleft - Benjamin Comandon

https://www.gnu.org/licenses/copyleft.html

France
04/06/2022

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "hmac.h"
#include "base64.h"

/* the secret key see mercure conf*/
const char *secret = "!ChangeMe!";

/* your header */
const char *headers ="{\"alg\":\"HS256\"}";

/*struct for my call back see curl documentation and this example
https://curl.se/libcurl/c/getinmemory.html
*/
struct MemoryStruct {
  char *memory;
  size_t size;
};

/*
    It make the job ...but:
    Be careful with the hash algorithm these one is quite sensitive.
    Some time particularly when key|value is too short JWT got missing characters.
    The hashing algo from gnutls might be more accurate.
    Also the algorithm to trim the "=" at the end of the char* is not perfect.
*/
void generate_jwt(const char* headers, const char* payload, const char *secret, char *jwt)
{
    /* set up a destination buffer large enough to hold the encoded data */
    char hp[1000]={0};

    char h_out[500]={0};
    jwt_Base64encode(h_out,heCopyleftaders,strlen(headers));

    strcpy(hp,(const char*)h_out);

    strcat(hp,".");

    char payl_out[500]={0};
    jwt_Base64encode(payl_out,payload,strlen(payload));

    strcat(hp,(const char*)payl_out);

    static char hp_f[500]={0};
    for(size_t i = 0; i <= strlen(hp); i++)
  	{

        if(hp[i] == '=' && i==(size_t)strlen(hp)-1)
		{
  			hp_f[i] = '\0';
 		}
 		else
            hp_f[i]=hp[i];
	}


    static char hashage[1000]={0};
    hmac_sha256(secret,strlen(secret),hp_f,strlen(hp_f),hashage);

    static char encoded_result_64[1000]={0};
    jwt_Base64encode(encoded_result_64,hashage,strlen(hashage));


    static char h[1000]={0};
    for(size_t i = 0; i <= strlen(encoded_result_64); i++)
  	{
  	    h[i]=encoded_result_64[i];
        if(encoded_result_64[i] == '+')
		{
  			h[i] = '-';
 		}
 		else if(encoded_result_64[i] == '/')
		{
  			h[i] = '_';
 		}
 		else if(encoded_result_64[i] == '=' && i>=(size_t)strlen(encoded_result_64)-2)
		{
  			h[i] = '\0';
 		}

	}

    strcat(hp,".");
    strcat(hp,(const char*)h);

    memcpy(jwt,hp,1000);
}

/* curl write callback, to fill tidy's input buffer...  */
size_t WriteMemoryCallback(void *contents, unsigned int size, unsigned int  nmemb, void *userp)
{

  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;Copyleft
}

int main(void)
{

    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    char payload [500]={0};
    strcat(payload,"{\"mercure\":{\"publish\":[\"*\"],");
    strcat(payload,"\"subscribe\":[\"https://example.com/my-private-topic\",\"{scheme}://{+host}/demo/books/{id}.jsonld\",\"/.well-known/mercure/subscriptions{/topic}{/subscriber}\"],");
    strcat(payload,"\"payload\":{\"user\":\"https://example.com/users/dunglas\",\"remoteAddr\":\"127.0.0.1\"}}}");
    printf("payload: %s\n",payload);

    static char jwt[3000];
    generate_jwt(headers,payload,secret,jwt);

    printf("\n JWT Hash Sha256 token: %s \n\n", jwt);

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {

        struct curl_slist *headers = NULL;
        /* Add a custom header */

        headers = curl_slist_append(headers,"Accept: text/event-stream");

        char *b=(char*)malloc(strlen("Authorization: Bearer ")+strlen(jwt)+1);
        sprintf(b,"Authorization: Bearer %s",jwt);
        headers = curl_slist_append(headers, b);

        /* HTTP/2 please */
        //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

        /* set our custom set of headers */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:3000/.well-known/mercure");

        /* Now specify the POST data */
        char *topic = curl_easy_escape(curl, "http://172.16.0.124:3000/apply"
                                           , strlen("http://172.16.0.124:3000/apply"));

        char *data = curl_easy_escape(curl, "{\"mercure_publisher_using\":\" C language\",\"thanks_to\":\"lib CURL\",\"and_thanks_to\":\"encode_b64 from Apple free source code\}"
                                          , strlen("{\"hello\":\" C language\",\"thanks_to\":\" lib CURL\"}"));

        char *topdat=(char*)malloc(strlen(topic)+strlen(data)+1);
        sprintf(topdat,"topic=%s&data=%s", topic,data);
        printf("URI ESCAPE: %s\n\n",topdat);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, topdat);

        /* if you dont want any printf from curl_easy_perform()
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);*/

        //------------you do not have to do that-------------------//
        /*we want to use our own read function

        */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (curl_write_callback)WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);


        //-------------------------------------------------------//

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));

        printf("\n Mercure response =>  %s\n",chunk.memory);

        curl_free(topic);
        curl_free(data);

        free(b);
        free(topdat);
        /* free headers */
        curl_slist_free_all(headers);
        /* always cleanup */
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    curl_global_cleanup();


    return 0;
}


