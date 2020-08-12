rm -f /usr/share/nginx/html/hackers_count/redirect.txt
touch /usr/share/nginx/html/hackers_count/redirect.txt
bash -c 'echo "MY_NUM redirections" > /usr/share/nginx/html/hackers_count/redirect.txt'
MY_NUM=$(cat /var/log/nginx/error.log | grep detected | tail -n 1 | awk '{print$2}')
echo $MY_NUM
sed -i -e "s/MY_NUM/$MY_NUM/g" /usr/share/nginx/html/hackers_count/redirect.txt
