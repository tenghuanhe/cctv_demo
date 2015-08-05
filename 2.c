#include <curl/curl.h>
#include <pillowtalk.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

void http_put_file(char* file, char* server_target);
static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream);

int main()
{
    char* server = "https://tenghuanhe:hetenghuan@tenghuanhe.cloudant.com/cctv11/1/1.jpg?rev=1-c181e911a2584dccf0c056f6265966b9";
    char* file = "1.jpg";

    http_put_file(file, server);
    return 0;
}

void http_put_file(char* file, char* server_target)
{
    FILE* fp;
    struct curl_slist* header_list = NULL;
    CURL* curl_handle;
    CURLcode res;
    struct stat file_info;

    stat(file, &file_info);
    fp = fopen(file, "rb");
    curl_handle = curl_easy_init();
    header_list = curl_slist_append(header_list, "Content-Type: image/jpeg");

    if (curl_handle)
    {
        curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_URL, server_target);
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
        curl_easy_setopt(curl_handle, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

        res = curl_easy_perform(curl_handle);
    }

    fclose(fp);
    curl_easy_cleanup(curl_handle);
}

static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t retcode;
    curl_off_t nread;

    retcode = fread(ptr, size, nmemb, stream);

    nread = (curl_off_t)retcode;
    fprintf(stderr, "***We read %" CURL_FORMAT_CURL_OFF_T" bytes from file\n", nread);

    return retcode;
}
