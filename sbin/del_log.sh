#!/bin/sh
#日志文件备份删除

#param:
#$1: log_path
#$2: log_name. eg:tars.log*
#$3: backup day
#$4: delete day
backup_del() {
	log_path=${1}
	log_name=${2}
	backup_day=${3}
	delete_day=${4}

	cd $log_path

	find -mtime +${backup_day} -type f -name "${log_name}" ! -name "*.gz" | xargs gzip
	find -mtime +${delete_day} -type f -name "${log_name}*.gz" | xargs rm -f
}

backup_del "${1}" "main_*.log" 7 20
