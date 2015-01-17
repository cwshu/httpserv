int main(int argc, char *argv[]){
    /* 
     * 1. start fork-based http server.
     * 2. recieve and parsing http request. 
     *    2.1. http method, path, version.
     *    2.2. header into map.
     *    2.3. content will be processed by file handler.
     * 3. process path, find a file existence and choose file handler.
     * 4. cgi and http file handler.
     * 5. http response (status code) ... etc.
     */
    return 0;
}
