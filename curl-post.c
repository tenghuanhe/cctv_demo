#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pillowtalk.h>


int main(void)
{
    const char* server_target = "https://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv4/_bulk_docs";
    int i;
    char* data = NULL;
    long data_len = 0;

    pt_node_t* bulk = pt_map_new();
    pt_node_t* docs = pt_array_new();
    pt_map_set(bulk, "docs", docs);


    for (i = 0; i < 10; i++)
    {
        pt_node_t* doc = pt_map_new();

        pt_node_t* geometry = pt_map_new();
        pt_node_t* coordinates = pt_array_new();
        pt_array_push_back(coordinates, pt_double_new((double)i));
        pt_array_push_back(coordinates, pt_double_new((double)(i + 10)));
        pt_map_set(geometry, "type", pt_string_new("Point"));
        pt_map_set(geometry, "coordinates", coordinates);

        pt_node_t* properties = pt_map_new();
        pt_map_set(properties, "name", pt_string_new("Beijing"));

        pt_map_set(doc, "type", pt_string_new("Feature"));
        pt_map_set(doc, "geometry", geometry);
        pt_map_set(doc, "properties", properties);
        pt_array_push_back(docs, doc);
    }

    if (bulk)
    {
        data = pt_to_json(bulk, 0);
        if (data)
            data_len = strlen(data);
    }

    CURL *curl_handle = curl_easy_init();
    struct curl_slist* header_list = NULL;

    header_list = curl_slist_append(header_list, "Content-Type: application/json");
    curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);

    curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);
    
    return 0;
}

