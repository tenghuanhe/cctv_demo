#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pillowtalk.h>


int main(void)
{
    const char* server_target = "https://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv10";
    int i;
    char* data = NULL;
    long data_len = 0;

    pt_node_t* bulk = pt_map_new();
    pt_map_set(bulk, "name", pt_string_new("IBM"));

    if (bulk)
    {
        data = pt_to_json(bulk, 2);

        if (data)
            data_len = strlen(data);
    }

    CURL *curl_handle = curl_easy_init();
    CURLcode res;
    struct curl_slist* header_list = NULL;

    header_list = curl_slist_append(header_list, "Content-Type: application/json");
    curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
//    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 3L);

    res = curl_easy_perform(curl_handle);
    printf("%d\n", res);
    curl_easy_cleanup(curl_handle);
    
    return 0;
}

