#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pillowtalk.h>


int main(void)
{
    const char* server_target = "http://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv10";
    int i;
    char* data = NULL;
    long data_len = 0;


    CURL *curl_handle = curl_easy_init();
    CURLcode res;
    curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);

    res = curl_easy_perform(curl_handle);
    printf("%d\n", res);
    curl_easy_cleanup(curl_handle);
    
    return 0;
}

