#/bin/bash

export MODULE;
MODULE+=" --add-module=src/lg_core"
MODULE+=" --add-module=src/lg_json"
MODULE+=" --add-module=src/lg_ocilib"
MODULE+=" --add-module=src/ad_system_display_module"
MODULE+=" --error-log-path=/ad_system/ad_system_familydoctor_jbk/log/nginx-display/wangmeng.log"
sh configure $MODULE && make

