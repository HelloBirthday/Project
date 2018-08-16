#include <fcntl.h>             // 提供open()函数  
#include <unistd.h>  
#include <stdio.h>  
#include <dirent.h>            // 提供目录流操作函数  
#include <string.h>  
#include <sys/stat.h>        // 提供属性操作函数  
#include <sys/types.h>         // 提供mode_t 类型  
#include <stdlib.h> 
#include <ctype.h>


#define USB_FOLDER_NAME "/sys/bus/usb/devices"
#define USB_PID_FILE_NAME "idProduct"
#define USB_VID_FILE_NAME "idVendor"


int result_usb_pid;
int result_usb_vid;

int htoi(char s[])  
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if(tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}  


int scan_file_id(char *filename)
{
	char pid_name[50];
	char vid_name[50];
	int pfd = -1;
	int vfd = -1;
	int ret = -1;
	char buf[20];
	int pid,vid;

	memset(pid_name, 0, 50*sizeof(char));
	memset(buf, 0, 20*sizeof(char));
    sprintf(pid_name,"%s%s%s",filename,"/",USB_PID_FILE_NAME); 

    //printf("%s: find <%s> USB VID and PID: %s\n",__func__,filename,pid_name); 

    pfd = open(pid_name,O_RDONLY);
    if(pfd < 0){
    	//printf("%s not found\n",pid_name);
    	return -1;
    }else{    //存在idProduct节点
    	ret = read(pfd,&buf,sizeof(buf));
    	if(ret > 0){
    		pid = htoi(buf);
    		if(pid == result_usb_pid){
    			memset(buf, 0, 20*sizeof(char));
    			memset(vid_name, 0, 50*sizeof(char));
				memset(buf, 0, 20*sizeof(char));
    			sprintf(vid_name,"%s%s%s",filename,"/",USB_VID_FILE_NAME); 
    			vfd = open(vid_name,O_RDONLY);
    			if(vfd > 0){
    				ret = read(vfd,&buf,sizeof(buf));
    				if(ret > 0){
    					vid = htoi(buf);
    					if(vid == result_usb_vid)
    					{
    						printf("%s:Find USB Product:%s\n",__func__,filename);
    						return 0;
    					}	
    				}
    			}
    		}
    	}
    }



	return -1;
}

int scan_dir(char *dir, int depth)   // 定义目录扫描函数  
{  
    DIR *dp;                      // 定义子目录流指针  
    struct dirent *entry;         // 定义dirent结构指针保存后续目录  
    struct stat statbuf;          // 定义statbuf结构保存文件属性  
    char dirname[50]; 
    int ret = -1; 

    
    if((dp = opendir(dir)) == NULL) // 打开目录，获取子目录流指针，判断操作是否成功  
    {  
        puts("can't open dir.");  
        return -1;  
    }  
    chdir (dir);                     // 切换到当前目录  
    while((entry = readdir(dp)) != NULL)  // 获取下一级目录信息，如果未否则循环  
    {  
        lstat(entry->d_name, &statbuf); // 获取下一级成员属性  
        if(S_IFDIR & statbuf.st_mode)    // 判断下一级成员是否是目录  
        {  
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)  
              continue;  
   
            scan_dir(entry->d_name, depth+4);              // 递归调用自身，扫描下一级目录的内容  
        }  
        else{   // sys/bus/usb/devices/ 下面都是链接文件，因此都不是目录
        	//printf("2222 %*s%s\n", depth, "", entry->d_name);  // 输出属性不是目录的成员 
        	memset(dirname, 0, 50*sizeof(char));
            sprintf(dirname,"%s%s%s",USB_FOLDER_NAME,"/",entry->d_name);  
            ret = scan_file_id(dirname);
        }  
            
    }  
    chdir("..");                                                  // 回到上级目录  
    closedir(dp);                                                 // 关闭子目录流  

    return ret;
}  

int main(int argc, char **argv)
{
    int ret = -1;


    if(argc < 2){
        result_usb_vid = 0x18D1;
        result_usb_pid = 0xDDDD;
    }else if(argc == 3){
        result_usb_vid = htoi(argv[1]);
        result_usb_pid = htoi(argv[2]);
        //printf("%s 0x%x:0x%x\n",argv[0],htoi(argv[1]),htoi(argv[2]));
    }else{
        fprintf(stderr, "Usage: %s VID PID \n\texample : %s 0x1a40 0x0201\n", argv[0],argv[0]);
        return 1;
    }


	ret = scan_dir(USB_FOLDER_NAME, 0); 
    if(ret < 0){
        printf("%s:Not Find USB VID:PID <%x:%x>\n",argv[0],result_usb_vid,result_usb_pid);
    }

	return  0;
}

//logcat -b radio | grep q3video