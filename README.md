# Mercure_C_Publisher
Publisher example for Mercure Server + JWT generator using C language with
libcurl.so ( https://curl.se/libcurl/ )
hmac | sha256 | encodeb64 source code (my own JWT generator)

First you need to configure your Mercure server locally
See: https://mercure.rocks/docs/hub/install

# Mercure Server

Personnally i use http://127.0.0.1:3000/.well-known/mercure/ui/

For "Topics to get updates for*" my topic is "http://172.16.0.124:3000/apply"
Have a look in my C# main, you're free to change it.

# To compile this C project 

I recommend using Code Block on Linux X86_64 cause libcurl.so is already compiled and embedded with the project.
No MakeFile at this moment

If there is any issues please leave a message

Enjoy !
