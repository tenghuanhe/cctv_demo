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

pt_node_t* get_gps_doc();
pt_node_t* get_log_doc();
pt_node_t* get_key_frame_doc();
static void* myrealloc(void* ptr, size_t size);
static size_t recv_memory_callback(void* ptr, size_t size, size_t nmemb, void* data);
void http_post_data(char* data, int data_len, char* server_target);
void upload_local_as_bulks(char* file, char* server_target);
void upload_a_single_doc(pt_node_t* doc, char* server_target);
void upload_keyframe_doc_with_attachment(pt_node_t* doc, char* filename);
void save_gps_to_local(pt_node_t* gps, char* file);

double getLon();
double getLat();
int getEngID();
int getGpsType();
int getOperationTypeID();
char* getDescription();
char* getTime();

int main(int argc, char** argv)
{
    char* server_target = "http://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv1";
    pt_node_t* gps_doc = get_gps_doc();
	pt_node_t* log_doc = get_log_doc();

    upload_a_single_doc(gps_doc, server_target);
    upload_a_single_doc(log_doc, server_target);
    return 0; 
}

pt_node_t* get_gps_doc()
{
	char* time = getTime();
	pt_node_t* gps_doc = pt_map_new();

	pt_map_set(gps_doc, "_id", pt_string_new(time));
	pt_map_set(gps_doc, "ENGID", pt_integer_new(getEngID()));
	pt_map_set(gps_doc, "GpsType", pt_integer_new(getGpsType()));
	pt_map_set(gps_doc, "Longtitude", pt_double_new(getLon()));
	pt_map_set(gps_doc, "Latitude", pt_double_new(getLat()));
	pt_map_set(gps_doc, "GpsTime", pt_string_new(time));

	return gps_doc;
}

pt_node_t* get_log_doc()
{
	char* time = getTime();
	pt_node_t* log_doc = pt_map_new();
	pt_node_t* gps = pt_map_new();

	pt_map_set(gps, "Longtitude", pt_integer_new(getLon()));
	pt_map_set(gps, "Latitude", pt_integer_new(getLat()));
	pt_map_set(gps, "GpsTime", pt_string_new(time));

	pt_map_set(log_doc, "ENGID", pt_integer_new(getEngID()));
	pt_map_set(log_doc, "GPS", gps);
	pt_map_set(log_doc, "OperationTypeID", pt_integer_new(getOperationTypeID()));
	pt_map_set(log_doc, "Description", pt_string_new(getDescription()));
	pt_map_set(log_doc, "OperationTime", pt_string_new(time));

	return log_doc;
}

pt_node_t* get_keyframe_doc()
{
	pt_node_t* key_frame_doc = pt_map_new();
	pt_node_t* attachment = pt_map_new();

	pt_map_set(key_frame_doc, "ENGID", pt_integer_new(getEngID()));
	pt_map_set(key_frame_doc, "_attachments", attachment);
}

double getLon()
{
    return 1.0;
}

double getLat()
{
    return 1.0;
}

int getEngID()
{
	return 1;
}

int getGpsType()
{
	return 1;
}

int getOperationTypeID()
{
	return 2;
}

char* getDescription()
{
	char* des= malloc(sizeof(char) * 256);
	des= "This is the description part";

	return des;
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
    data = pt_to_json(doc, 2);
    data_len = strlen(data);

	printf("%s\n", data);

    http_post_data(data, data_len, server_target);
}

void save_gps_to_local(pt_node_t* gps, char* file)
{
    char* data = NULL;
    int data_len = 0;
    data = pt_to_json(gps, 0);
    data_len = strlen(data);
}

void upload_keyframe_doc_with_attachment(pt_node_t* doc, char* filename)
{
	
}
