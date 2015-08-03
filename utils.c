#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pillowtalk.h>

struct memory_chunk
{
    char* memory;
    char* offset;
    size_t size;
};

pt_node_t* pt_gps(double lon, double lat);
void http_post_data(char* data, int data_len, const char* server_target);
static void* myrealloc(void* ptr, size_t size);
static size_t recv_memory_callback(void* ptr, size_t size, size_t nmemb, void* data);
double getLon();
double getLat();
char* getTime();

int main(int argc, char** argv)
{
    const char* server_target = "https://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv";
    pt_node_t* gps = pt_gps(getLon(), getLat());
    char* data = NULL;
    int data_len = 0;

    data = pt_to_json(gps, 0);
    data_len = strlen(data);

    http_post_data(data, data_len, server_target);
    return 0; 
}

pt_node_t* pt_gps(double lon, double lat)
{
    pt_node_t* doc = pt_map_new();

    pt_node_t* geometry = pt_map_new();
    pt_node_t* coordinates = pt_array_new();
    pt_array_push_back(coordinates, pt_double_new(lon));
    pt_array_push_back(coordinates, pt_double_new(lat));
    pt_map_set(geometry, "type", pt_string_new("Point"));
    pt_map_set(geometry, "coordinates", coordinates);

    pt_node_t* properties = pt_map_new();
    pt_map_set(properties, "time", pt_string_new(getTime()));

    pt_map_set(doc, "type", pt_string_new("Feature"));
    pt_map_set(doc, "geometry", geometry);
    pt_map_set(doc, "properties", properties);

    return (pt_node_t*)doc;
}

double getLon()
{
    return 1.0;
}

double getLat()
{
    return 1.0;
}

char* getTime()
{
    char* string = malloc(sizeof(char) * 20);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(string, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return (char*)string;
}

void http_post_data(char* data, int data_len, const char* server_target)
{
    struct curl_slist* header_list = NULL;
    struct memory_chunk chunk;
    CURL *curl_handle = curl_easy_init();
    CURLcode res;
    chunk.memory = NULL;
    chunk.size = 0;

    header_list = curl_slist_append(header_list, "Content-Type: application/json");
    curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, recv_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

    res = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);
}

static void* myrealloc(void* ptr, size_t size)
{
    if(ptr)
        return (void*)realloc(ptr, size);
    else
        return (void*)malloc(size);
}

static size_t recv_memory_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
    size_t realsize = size * nmemb;
    struct memory_chunk *mem = (struct memory_chunk*)data;

    mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory)
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}
