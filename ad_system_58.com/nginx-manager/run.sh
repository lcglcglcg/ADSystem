#/bin/bash
make clean
./configure --add-module=src/ad_system_manager_module --with-debug --error-log-path=/ad_system/ad_system_58.com/log/nginx-manager/wangmeng.log
make
