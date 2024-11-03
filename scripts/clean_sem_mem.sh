#!/bin/zsh

# 删除所有共享内存段
ipcs -m | awk 'NR > 3 {print $2}' | while read -r shmid
do
    if [[ -n "$shmid" ]]; then
        ipcrm -m "$shmid"
    fi
done

# 删除所有信号量
ipcs -s | awk 'NR > 3 {print $2}' | while read -r semid
do
    if [[ -n "$semid" ]]; then
        ipcrm -s "$semid"
    fi
done

echo "已删除所有共享内存和信号量"
