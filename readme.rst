httpserv: Simple HTTP Server
----------------------------

example usage
~~~~~~~~~~~~~
::

    make 
    ./build/httpd 8000 testing/docroot1/
    # ./build/httpd <port> <document_root>

tests
~~~~~
1. static object, HTTP response status code
::

    touch testing/docroot1/no_permission
    chmod a-r testing/docroot1/no_permission

    ./build/httpd 8000 testing/docroot1/

    # if we have installed netcat (nc), only tests in OpenBSD version netcat currently.
    ./testing/script1/run.sh 8000

2. static object, mine type, directory
::

    mkdir testing/docroot2/no_perm_dir
    chmod a-r testing/docroot2/no_perm_dir
    mkdir testing/docroot2/no_perm_index
    touch testing/docroot2/no_perm_index/index.html
    chmod a-r testing/docroot2/no_perm_index/index.html
    
    ./build/httpd 8000 testing/docroot2/

    # testcases
    GET /plain.txt?t=1465979599
    GET /simple.html?t=1465979599
    GET /image.png?t=1465979599
    GET /audio.ogg?t=1465979599
    GET /video.mp4?t=1465979599
    GET /missing?t=1465979599
    GET /noperm?t=1465979599
    GET /dir1?t=1465979599
    GET /dir2?t=1465979599

    GET /no_perm_index?t=1465979599
    GET /no_perm_dir?t=1465979599

