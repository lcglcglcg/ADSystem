#/bin/bash
make clean
./configure --add-module=src/ad_system_manager_module --with-debug --error-log-path=/ad_system/ad_system_familydoctor_yyk/log/nginx-manager/wangmeng.log
make
