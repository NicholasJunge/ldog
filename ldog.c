/********************************************************************************
 *项目： 日志文件监控程序
 *版本： 1.0.2
 *需求： 能用于后台日志监控，便于功能测试即可
 *改进： 改进bug, Segment Fault
 *设计： 欣赏一下俊哥如何阐释C语言的debug风格，嘿嘿~~
 *作者： Nicolas.Junge
 *时间： 2016-11-30 19:47
 *备注： 今天闯红灯被抓了，郁闷啊，晚上买的好的吃，消化一下郁闷的心情^~^
 **********************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>


#define LOG_COUNT 41		//所要监控的日志数量
#define REAL_LOG_COUNT 41	//实际日志数量，可能小于LOG_COUNT
#define READ_BUF 65536		//原来使用8k报错：Segment Fault


int changeid = 0;			//打印变化内容时，用于标志序列
void init_fp();				//函数声明，将需要的文件路径加载进来 
void print_baseinfo();		//打印文件基础信息
void close_fd();			//close file description
void update_stat();			//update the file stat
void show_change();			//show changes content


struct LOG_NODE_INFO { 
	char fp[256];			//file path, 文件路径
	int fd;					//文件描述符
	struct stat sb;			//文件状态表
	int uid;				//update id, 节点更新标志：从0开始增长，代表更新次数
}log_node[LOG_COUNT];		//定义256个文件,用于监控



int main(int argc, char *argv[])	//参数控制程序的功能，第二阶段实现。
{
	
	int i = 0;		
	char s_input[3] = {0};		//用户选项: -h -u	

	init_fp();											//初始化文件路径，为后续获取文件状态准备
	for(i = 0; i < REAL_LOG_COUNT; i++){				//根据给出的日志路径个数，初始化文件描述符和文件状态表
		log_node[i].fd = open(log_node[i].fp, O_RDONLY);
		fstat(log_node[i].fd, &(log_node[i].sb));
		log_node[i].uid = 0;
	}
	
	while(1){
		if (argc == 2){
			printf("Input [p] to print\nInput [u] to update\nInput [s] to show changes\n");
			scanf("%s", &s_input);
			if (0 == strcmp(s_input, "p")) {
				print_baseinfo();
			}
			if (0 == strcmp(s_input, "u")) {					//update the files stat
				update_stat();
				print_baseinfo();
			}
			if (0 == strcmp(s_input, "s")) {					//show changes content
				show_change();	
			}
		}else{
			show_change();
		}		
	}	

	close_fd();					
	return 0;	
}

void update_stat()										//update the file stat
{
	int i = 0;
	for(; i < REAL_LOG_COUNT; i++){						//根据给出的日志路径个数，初始化文件描述符和文件状态表
		stat(log_node[i].fp, &(log_node[i].sb));
		log_node[i].uid++;
	}
}
void show_change()
{
	int i = 0;
	int ret = 0;
	struct LOG_NODE_INFO log_node_update;
	char buf[READ_BUF] = {0};
	while(1){
		for(i = 0; i < REAL_LOG_COUNT; i++){													//根据给出的日志路径个数，初始化文件描述符和文件状态表
			ret = 0;
			ret = stat(log_node[i].fp, &(log_node_update.sb));										//获取更新后的文件状态表
			if (ret != 0){
			//	perror("stat\n");
			//	printf("File:%s\n", log_node[i].fp);
				continue;
			}	
			if(log_node_update.sb.st_size != log_node[i].sb.st_size){							//判断文件大小是否发生变化
				changeid++;
				printf("[Change %d] %s is changed, size is %ld bytes\n", changeid, log_node[i].fp, log_node_update.sb.st_size);
				//file has changed, then print the changes
				lseek(log_node[i].fd, log_node[i].sb.st_size, SEEK_SET);
				read(log_node[i].fd, buf, log_node_update.sb.st_size - log_node[i].sb.st_size);
				log_node[i].sb.st_size = log_node_update.sb.st_size;
				log_node[i].uid++;
				printf("[Content] %s\n", buf);
			}else{
				usleep(200);
			}
		}
	}
}
void print_baseinfo()
{
	int i = 0;
	struct tm *tm_fm;
	char time_buf[32] = {0};

	for(; i < REAL_LOG_COUNT; i++){
		tm_fm = localtime(&(log_node[i].sb.st_mtime));
		strftime(time_buf, 32, "%Y-%m-%d %H:%M:%S", tm_fm);
		printf("[编号:\t%d\tsize:\t%d bytes\t当前更新次数:\t%d\t文件更新时间:%s]\t[路径:%s]\n", i, log_node[i].sb.st_size, log_node[i].uid, time_buf, log_node[i].fp);
	}
}

void close_fd()
{
	int i = 0;
	
	for(; i < REAL_LOG_COUNT; i++){
		close(log_node[i].fd);
	}
}

void init_fp()
{
	strcpy(log_node[0].fp, "/home/leagsoft/LeagView/Apache/bin/Log/TransferService.log");
	strcpy(log_node[1].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/RestService.log");
	strcpy(log_node[2].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/emm-api.log");
	strcpy(log_node[3].fp, "/home/leagsoft/LeagView/tomcat-uai/bin/Log/emm-manager.log");
	strcpy(log_node[4].fp, "/home/leagsoft/LeagView/log/LVMainServer_0.log");
	strcpy(log_node[5].fp, "/home/leagsoft/LeagView/tomcat-uai/bin/stderr.log");
	strcpy(log_node[6].fp, "/home/leagsoft/LeagView/tomcat-uai/bin/stdout.log");
	strcpy(log_node[7].fp, "/home/leagsoft/LeagView/tomcat-uai/bin/Log/emm-manager.log");
	strcpy(log_node[8].fp, "/home/leagsoft/LeagView/tomcat-uai/bin/webserver.log");
	strcpy(log_node[9].fp, "/home/leagsoft/LeagView/tomcat-uai/logs/catalina.2016-11-30.log");
	strcpy(log_node[10].fp, "/home/leagsoft/LeagView/tomcat-uai/logs/localhost.2016-11-30.log");
	strcpy(log_node[11].fp, "/home/leagsoft/LeagView/tomcat-uai/logs/host-manager.2016-11-30.log");
	strcpy(log_node[12].fp, "/home/leagsoft/LeagView/tomcat-uai/logs/manager.2016-11-30.log");
	strcpy(log_node[13].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/RestService.log.2016-11-30_00.log");
	strcpy(log_node[14].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/emm-api.log.2016-11-30_00.log");
	strcpy(log_node[15].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/emm-api.log");
	strcpy(log_node[16].fp, "/home/leagsoft/LeagView/tomcat-service/bin/Log/server.log");
	strcpy(log_node[17].fp, "/home/leagsoft/LeagView/tomcat-service/bin/log/smartice.log");
	strcpy(log_node[18].fp, "/home/leagsoft/LeagView/tomcat-service/bin/webserver.log");
	strcpy(log_node[19].fp, "/home/leagsoft/LeagView/tomcat-service/logs/host-manager.2016-11-30.log");
	strcpy(log_node[20].fp, "/home/leagsoft/LeagView/tomcat-service/logs/catalina.2016-11-30.log");
	strcpy(log_node[21].fp, "/home/leagsoft/LeagView/tomcat-service/logs/localhost.2016-11-30.log");
	strcpy(log_node[22].fp, "/home/leagsoft/LeagView/tomcat-service/logs/host-manager.2016-11-30.log");
	strcpy(log_node[23].fp, "/home/leagsoft/LeagView/tomcat-service/logs/manager.2016-11-30.log");
	strcpy(log_node[24].fp, "/home/leagsoft/LeagView/License/GenerateLicense_0.log");
	strcpy(log_node[25].fp, "/home/leagsoft/LeagView/License/TestLicense_0.log");
	strcpy(log_node[26].fp, "/home/leagsoft/LeagView/Apache/logs/host-manager.2016-11-30.log");
	strcpy(log_node[27].fp, "/home/leagsoft/LeagView/Apache/logs/localhost.2016-11-30.log");
	strcpy(log_node[28].fp, "/home/leagsoft/LeagView/Apache/logs/manager.2016-11-30.log");
	strcpy(log_node[29].fp, "/home/leagsoft/LeagView/Apache/logs/tunnel.log");
	strcpy(log_node[30].fp, "/home/leagsoft/LeagView/Apache/webapps/TunnelGateway/lua/cc.log");
	strcpy(log_node[31].fp, "/home/leagsoft/LeagView/tcp_proxy/log/ngx_tcp_proxy_0.log");
	strcpy(log_node[32].fp, "/home/leagsoft/LeagView/tcp_proxy/log/SI-ngx_tcp_proxy_0.log");
	strcpy(log_node[33].fp, "/home/leagsoft/LeagView/Bin/BSLOG_0.log");
	strcpy(log_node[34].fp, "/home/leagsoft/LeagView/log/SI-LVMainServer_1.log");
	strcpy(log_node[35].fp, "/home/leagsoft/LeagView/log/SI-CheckLicense_0.log");
	strcpy(log_node[36].fp, "/home/leagsoft/LeagView/log/SIStat-CheckLicense_0.log");
	strcpy(log_node[37].fp, "/home/leagsoft/LeagView/log/SIHistory-LVMainServer_0.log");
	strcpy(log_node[38].fp, "/usr/local/nginx/logs/error.log");
	strcpy(log_node[39].fp, "/usr/local/nginx/logs/ssl-access.log");
	strcpy(log_node[40].fp, "/usr/local/nginx/logs/ssl-error.log");
}
