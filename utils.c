#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <pillowtalk.h>

struct memory_chunk
{
    char* memory;
    char* offset;
    size_t size;
};

pt_node_t* pt_gps(double lon, double lat);
static void* myrealloc(void* ptr, size_t size);
static size_t recv_memory_callback(void* ptr, size_t size, size_t nmemb, void* data);
void http_post_data(char* data, int data_len, char* server_target);
void upload_local_as_bulks(char* file, char* server_target);
void upload_a_single_doc(pt_node_t* doc, char* server_target);
void save_gps_to_local(pt_node_t* gps, char* file);

double getLon();
double getLat();
char* getTime();

int main(int argc, char** argv)
{
    char* server_target = "https://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv5";
    pt_node_t* gps = pt_gps(getLon(), getLat());

    upload_a_single_doc(gps, server_target);
    upload_local_as_bulks("pp.json", server_target);
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
    char* string = malloc(sizeof(char) * 100);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(string, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return (char*)string;
}

void http_post_data(char* data, int data_len, char* server_target)
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
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, recv_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

    res = curl_easy_perform(curl_handle);
    printf("%s\n", chunk.memory);
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

void upload_local_as_bulks(char* file, char* server_target)
{
    FILE* fp = NULL;
    struct stat st;

    const char* bulk_head = "{\"docs\": [";
    const char* bulk_tail = "]}";
    const char* bulk_docs = "/_bulk_docs";
    char* server_bulk_target;
    char* file_data;
    char* data;
    int data_len;
    int file_data_len;
    int server_len;

    fp = fopen(file, "r");
    if(fp == NULL)
        return;
    
    fstat(fileno(fp), &st);

    // Here subtraction by 2 to remove the tailling comma in file
    file_data = malloc(sizeof(char) * st.st_size);
    file_data_len = st.st_size - 2;
    fread(file_data, sizeof(char), st.st_size, fp);
    data_len = file_data_len + strlen(bulk_head) + strlen(bulk_tail);
    data = malloc(data_len);
    memcpy(data, bulk_head, strlen(bulk_head));
    memcpy(data + strlen(bulk_head), file_data, file_data_len);
    memcpy(data + strlen(bulk_head) + file_data_len, bulk_tail, strlen(bulk_tail));

    server_len = strlen(server_target) + strlen(bulk_docs);
    server_bulk_target = malloc(sizeof(char) * server_len);
    memcpy(server_bulk_target, server_target, strlen(server_target));
    memcpy(server_bulk_target + strlen(server_target), bulk_docs, strlen(bulk_docs));

    http_post_data(data, data_len, server_bulk_target);
}

void upload_a_single_doc(pt_node_t* doc, char* server_target)
{
    char* data = NULL;
    int data_len = 0;
    data = pt_to_json(doc, 0);
    data_len = strlen(data);

    http_post_data(data, data_len, server_target);
}

void save_gps_to_local(pt_node_t* gps, char* file)
{
    char* data = NULL;
    int data_len = 0;
    data = pt_to_json(gps, 0);
    data_len = strlen(data);

}
