//
//  download.c
//  tsschecker
//
//  Created by tihmstar on 07.01.16.
//  Copyright © 2016 tihmstar. All rights reserved.
//

#include <curl/curl.h>
#include "download.h"

size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    return fwrite(buffer, size, nmemb, stream);
}

int downloadFile(const char *url, const char *dstPath){
    debug("[DOWN] downloading file %s\n",url);
    CURL *mcurl = curl_easy_init();
    
    if (!mcurl) return CURLE_FAILED_INIT;
    FILE *dfile = fopen(dstPath, "wb");
    if (!dfile) {
        curl_easy_cleanup(mcurl);
        return CURLE_WRITE_ERROR;
    }
    
    curl_easy_setopt(mcurl, CURLOPT_URL, url);
    curl_easy_setopt(mcurl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(mcurl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(mcurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(mcurl, CURLOPT_WRITEFUNCTION, my_fwrite);
    curl_easy_setopt(mcurl, CURLOPT_WRITEDATA, dfile);
    curl_easy_setopt(mcurl, CURLOPT_FAILONERROR, 1);
    
    CURLcode res = curl_easy_perform(mcurl);
    curl_easy_cleanup(mcurl);
    if (fclose(dfile) && res == CURLE_OK) res = CURLE_WRITE_ERROR;
    if (res != CURLE_OK){
        error("failed to download file from=%s to=%s CURLcode=%d\n",url,dstPath,res);
        return res;
    }
    return 0;
}
