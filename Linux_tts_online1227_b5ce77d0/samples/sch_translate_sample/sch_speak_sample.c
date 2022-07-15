/*
* 语音语义技术能够将语音听写业务中的内容进行语义解析。
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "json.h"

#define	BUFFER_SIZE	4096
#define FRAME_LEN	640 
#define HINTS_SIZE  100

int json_pase(char * rec_result)
{
	enum json_error error;
	json_t * root          = NULL;
	json_t * resultjson    = NULL;
	json_t * sub_result    = NULL; 
	char   * originalstr   = NULL;
	int      ret           = 0;
	error = json_parse_document(&root,rec_result);
	if (error!=JSON_OK)
	{
		printf("\njson_parse_document failed, error code: %d\n", error);
		goto json_error;
	}
	resultjson = json_find_first_label(root,"trans_result");
	if (resultjson == NULL)
	{
		resultjson = json_find_first_label(root,"errmsg");
		if (resultjson!=NULL&&resultjson->child!=NULL)
		{
			originalstr = resultjson->child->text;
			printf("\n\n%s\n\n",originalstr);
		}
		else
		{
			printf("\njson_find_first_label failed,\"error\" not found\n");
			goto json_error;
		}
	}
	else
	{
		sub_result = json_find_first_label(resultjson->child,"dst");
		originalstr = sub_result->child->text;
		printf("\n\n%s\n\n",originalstr);
	}
	goto json_exit;

json_error:
	ret = -1;

json_exit:
	if (root!=NULL)
	{
		json_free_value(&root);
		root = NULL;
	}
	return ret;
}

void run_iat(const char* audio_file, const char* session_begin_params)
{
	const char*		session_id					=	NULL;
	char			*rec_result		            =	NULL;	
	char			hints[HINTS_SIZE]			=	{NULL}; //hintsÎª½áÊø±¾´Î»á»°µÄÔ­ÒòÃèÊö£¬ÓÉÓÃ»§×Ô¶¨Òå
	unsigned int	total_len					=	0; 
	int				aud_stat					=	MSP_AUDIO_SAMPLE_CONTINUE ;		//音频状态
	int				ep_stat						=	MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
	int				rec_stat					=	MSP_REC_STATUS_SUCCESS ;		//识别状态
	int				errcode						=	MSP_SUCCESS ;

	FILE*			f_pcm						=	NULL;
	char*			p_pcm						=	NULL;
	long			pcm_count					=	0;
	long			pcm_size					=	0;
	long			read_size					=	0;

	
	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm) 
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}
	
	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm); //获取音频文件大小 
	fseek(f_pcm, 0, SEEK_SET);		

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); //读取音频文件内容
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}
	
	session_id = QISRSessionBegin(NULL, session_begin_params, &errcode); //听写不需要语法，第一个参数为NULL
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
		goto iat_exit;
	}
	
	rec_result = (char*)malloc(BUFFER_SIZE);
	memset(rec_result,0,BUFFER_SIZE);

	while (1) 
	{
		unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
		int ret = 0;

		if (pcm_size < 2 * len) 
			len = pcm_size;
		if (len <= 0)
			break;

		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		printf(">");
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
		{
			printf("\nQISRAudioWrite failed! error code:%d\n", ret);
			goto iat_exit;
		}
			
		pcm_count += (long)len;
		pcm_size  -= (long)len;
		
		if (MSP_REC_STATUS_SUCCESS == rec_stat) //已经有部分听写结果
		{
			const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
			{
				printf("\nQISRGetResult failed! error code: %d\n", errcode);
				goto iat_exit;
			}
			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
				{
					printf("\nno enough buffer for rec_result !\n");
					goto iat_exit;
				}
				strncat(rec_result, rslt, rslt_len);
			}
		}

		if (MSP_EP_AFTER_SPEECH == ep_stat)
			break;
		usleep(200*1000); //模拟人说话时间间隙。200ms对应10帧的音频
	}
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRAudioWrite failed! error code:%d \n", errcode);
		goto iat_exit;	
	}

	while (MSP_REC_STATUS_COMPLETE != rec_stat) 
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRGetResult failed, error code: %d\n", errcode);
			goto iat_exit;
		}
		if (NULL != rslt)
		{
			unsigned int rslt_len = strlen(rslt);
			total_len += rslt_len;
			if (total_len >= BUFFER_SIZE)
			{	
				rec_result = (char*)realloc(rec_result,total_len);
				if (rec_result==NULL)
				{
					printf("\nno enough buffer for rec_result !\n");
					goto iat_exit;
				}
			}
			strncat(rec_result, rslt, rslt_len);
		}
		usleep(150*1000); //防止频繁占用CPU
	}
	
	errcode = json_pase(rec_result);
	if (errcode!=0)
	{
		printf("json_pase failed ,errcode is %d\n",errcode);
	}

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{	free(p_pcm);
		p_pcm = NULL;
	}
	if (rec_result!=NULL)
	{
		free(rec_result);
		rec_result = NULL;
	}
	QISRSessionEnd(session_id, hints);
}

int main(int argc, char* argv[])
{
	int			ret						=	MSP_SUCCESS;
	int			upload_on				=	1; //是否上传用户词表
	const char* login_params			=	"appid = b5ce77d0"; // 登录参数，appid与msc库绑定,请勿随意改动


	/*
	* sub:				请求业务类型
	* domain:			领域
	* language:			语言
	* accent:			方言
	* sample_rate:		音频采样率
	* result_type:		识别结果格式
	* result_encoding:	结果编码格式
	*
	* nlp_version:      语义版本
	* sch:              是否使用语义标识
	*/
	const char* session_begin_params ="nlp_version =3.0,trssrc=its,addcap = translate,orilang = en,translang = cn,scene = main,sch=1,sub=iat,domain = iat, language = zh_cn, accent = mandarin,aue = speex-wb;7, sample_rate = 16000, result_type = plain, result_encoding = utf8";

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		goto exit; //登录失败，退出登录
	}
	run_iat("wav/weather.pcm", session_begin_params);
exit:
	printf("按任意键退出 ...\n");
	getchar();
	MSPLogout(); //退出登录

	return 0;
}
