cd build;ninja;cd ..;
scp bin/web_ui root@192.168.1.$1:/mnt/UDISK/vslam/web2ui;
scp lib/libWeb.so root@192.168.1.$1:/usr/lib/.
