#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include<vector>
#include "vad.h"
#ifndef nullptr
#define nullptr 0
#endif
using namespace std;
struct WavData{
	public:
		int16_t* data;
		long size;
		
		WavData(){
			data=NULL;
			size=0;
		}	
};
#define BLOCKLENGTH 600000
typedef struct Sentence
{
 int beginTime;
 int endTime;
 unsigned int sentenceAudioSize;
 int16_t sentenceAudio[BLOCKLENGTH];
}Sentence;
struct Sentence sen[10];
typedef std::vector<Sentence> sentenceVec;
void loadWavFile(const char* fname,WavData *ret){
	FILE* fp=fopen(fname,"rb");
	if(fp){
		char id[5];
	        int32_t size;
		int16_t format_tag,channels,block_align,bits_per_sample;
		int32_t format_length,sample_rate,avg_bytes_sec,data_size;		
		
		fread(id,sizeof(char),4,fp);
		id[4]='\0';
		
		if(!strcmp(id,"RIFF")){
			fread(&size,sizeof(int16_t),2,fp);
			fread(id,sizeof(char),4,fp);
			id[4]='\0';
 
			if(!strcmp(id,"WAVE")){
				fread(id,sizeof(char),4,fp);
				fread(&format_length,sizeof(int16_t),2,fp);
				fread(&format_tag,sizeof(int16_t),1,fp);
				fread(&channels,sizeof(int16_t),1,fp);
				fread(&sample_rate,sizeof(int16_t),2,fp);
				fread(&avg_bytes_sec,sizeof(int16_t),2,fp);
				fread(&block_align,sizeof(int16_t),1,fp);
				fread(&bits_per_sample,sizeof(int16_t),1,fp);
				fread(id,sizeof(char),4,fp);
				fread(&data_size,sizeof(int16_t),2,fp);

				ret->size=data_size/sizeof(int16_t);
				// 动态分配了空间，记得要释放
				ret->data=(int16_t*)malloc(data_size);
				fread(ret->data,sizeof(int16_t),ret->size,fp);
			}
			else{
				cout<<"Error: RIFF File but not a wave file\n";
			}
		}
	else{
		cout<<"ERROR: not a RIFF file\n";
	}
	}
	fclose(fp);
}
 
void freeSource(WavData* data){
	free(data->data);
}
//bool vad(unsigned short sampleRate, char* audio, unsigned int audioSize, std::vector<Sentence>*  sentenceVec);
bool vadProcess(int16_t *buffer, uint16_t sampleRate, size_t samplesCount , std::vector<Sentence>  sentenceVec)
{
	int16_t vad_mode=1;
        int per_ms_frames=30;
	if (buffer == nullptr) return -1;
	if (samplesCount == 0) return -1;
	size_t samples = sampleRate * per_ms_frames / 1000;//每一帧有多少采样点
	if (samples == 0) return -1;
	int16_t *input = buffer;
	size_t nTotal = (samplesCount / samples);//总帧数
	printf("总帧数%d\n",nTotal);
        printf("samplesgcw+=%d\n",samples);
	VadInst* vadInst = WebRtcVad_Create();
	if (vadInst == NULL) return -1;	
	int status = WebRtcVad_Init(vadInst);
	if (status != 0) {
		printf("WebRtcVad_Init fail\n");
		WebRtcVad_Free(vadInst);
		return -1;
	}
	status = WebRtcVad_set_mode(vadInst, vad_mode);
	if (status != 0) {
		printf("WebRtcVad_set_mode fail\n");
		WebRtcVad_Free(vadInst);
		return -1;
	}
	//printf("Activity ： \n");
	int audio_vadarr[nTotal],a[nTotal];
	for (int i = 0; i < nTotal; i++) {
		int keep_weight = 0;
		int nVadRet = WebRtcVad_Process(vadInst, sampleRate, input, samples, keep_weight);
		if (nVadRet == -1) {
			printf("failed in WebRtcVad_Process\n");
			WebRtcVad_Free(vadInst);
			return -1;
		}
		else {
			printf(" %d \t", nVadRet);
			audio_vadarr[i] = nVadRet;//将预测结果放到数组中，方便处理
		}
		input += samples;
	}
	printf("num_total==%d\n",nTotal);
	WebRtcVad_Free(vadInst);
	//规则
	int i,temp=0,max_l=0,num_begin=0,num_end=0,begin_time[100],end_time[100],all_key=0,all_nokey=0;
	for(i=0; i<nTotal; i++)
	{   
		a[i] = audio_vadarr[i] ;//初始化
	}
	for(i=0; i<100; i++)
        {
                begin_time[i] = 0 ;//初始化
                end_time[i] = 0 ;
        }
	int length = nTotal;
	int key = 0;


    	for(i=0; i<length; i++){
        	if(a[i] == key )
        	{
            		temp++;
            		all_key++;
	//		printf("temp%d\n",temp);
            		if ( a[i+1] != key || (i+1) >= length )
            		{
                		if (temp > 5)
                		{
                    			if (i-temp>=0 && a[i-temp] != 0)
                    			{
                        			//printf("gcwend%d\n",i-temp);
                        			//printf("gcwi%d\n",i);
                        			//printf("gcwtemp%d\n",temp);
                        			end_time[num_end]=i-temp;
                        			num_end++;
                    			}
                    			if (a[i+1] != 0 && i+1 < length)
                    			{
                        			//printf("gcwbegin %d\n",i);
                        			begin_time[num_begin]=i;
                        			num_begin++;
                    			}
                    			temp = 0;
                    			max_l++;
                    			printf("\n");
                		}
            		}
            		else
            		{
                		if(temp > 5 && a[i+1] != key )
                		{
                    			//printf("end%d\n",i-temp);
                    			//printf("i%d\n",i);
                    			//printf("temp%d\n",temp);
                    			end_time[num_end]=i-temp;
                    			num_end++;
					temp = 0;
                		}
            		}
        	}
        	else
        	{
            		all_nokey++;
            		temp=0;
        	}
    	}
    	//push_back
    	printf("\n");
    	printf("num_begin%d\n",num_begin);
    	printf("num_end%d\n",num_end);
	int16_t *audio_save = buffer;
    	for(i=0; i<10; i++)
    	{
        	if(all_key == length){printf("全是非语音\n");break;}
        	if(all_nokey == length){printf("全是语音\n");break;}

        	if(i <num_begin && num_begin == num_end && begin_time[0] < end_time[0])//情况1
        	{
            		sen[i].beginTime = begin_time[i]*per_ms_frames;
            		sen[i].endTime = (end_time[i]+1)*per_ms_frames;
			sen[i].sentenceAudioSize = ((end_time[i]+1) - begin_time[i] )* samples;
	
			for(int j=(begin_time[i]*samples);j<(end_time[i]+1)*samples;j++){
				sen[i].sentenceAudio[j] =(int)audio_save[j];
				printf("点%d\n",sen[i].sentenceAudio[j]);
    			}

			printf("begin_Time%d\n",sen[i].beginTime);
			printf("end_Time%d\n",sen[i].endTime);
			printf("数目%d\n",sen[i].sentenceAudioSize);
	//		sentenceVec->push_back(sen[i]);
            		printf("情况1");
        	}
        	if(i < num_begin && (num_begin-1) == num_end && begin_time[0] < end_time[0])//情况2
        	{
            		printf("情况2");
            		if(i == (num_begin-1))
            		{
                		sen[i].beginTime = begin_time[i]*per_ms_frames;
                		sen[i].endTime = -1;
				sen[i].sentenceAudioSize =samplesCount - (begin_time[i] * samples);
				for(int j=(begin_time[i]*samples);j<samplesCount;j++){
        	                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
	                        }
		//              sentenceVec->push_back(sen[i]);

	                        printf("begin_Time%d\n",sen[i].beginTime);
        	                printf("end_Time%d\n",sen[i].endTime);
                	        printf("数目%d\n",sen[i].sentenceAudioSize);


            		}
            		else
            		{
               			printf("情况2");
               			sen[i].beginTime = begin_time[i]*per_ms_frames;
               			sen[i].endTime = (end_time[i]+1)*per_ms_frames;
				sen[i].sentenceAudioSize = ((end_time[i]+1) - begin_time[i] ) * samples;
				for(int j=(begin_time[i]*samples);j<(end_time[i]+1)*samples;j++){
        	                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
	                        }

		//              sentenceVec->push_back(sen[i]);
                        	printf("begin_Time%d\n",sen[i].beginTime);
                		printf("end_Time%d\n",sen[i].endTime);
		                printf("数目%d\n",sen[i].sentenceAudioSize);



            		}
        	}

        	if(i < (num_begin+1) && num_begin == num_end && begin_time[0] > end_time[0])//情况3
        	{
            		printf("情况3\n");
            		if(i == 0)
            		{
                		sen[i].beginTime = -1;
                		sen[i].endTime = (end_time[i]+1)*per_ms_frames;				
				sen[i].sentenceAudioSize = (end_time[i]+1) * samples;
				for(int j = 0;j<(end_time[i]+1)*samples;j++){
                                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
                                }

		//              sentenceVec.push_back(sen[i]);
				printf("begin_Time%d\n",sen[i].beginTime);
                                printf("end_Time%d\n",sen[i].endTime);
                                printf("数目%d\n",sen[i].sentenceAudioSize);
				printf("33333\n");
            		}
            		if(i == num_begin)
            		{
               			sen[i].beginTime = begin_time[i-1]*per_ms_frames;
               			sen[i].endTime = -1;
				sen[i].sentenceAudioSize =samplesCount - (begin_time[i-1] * samples);
				for(int j=(begin_time[i]*samples);j<samplesCount;j++){
                                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
                                }

		//              sentenceVec.push_back(sen[i]);
				printf("begin_Time%d\n",sen[i].beginTime);
                                printf("end_Time%d\n",sen[i].endTime);
                                printf("数目%d\n",sen[i].sentenceAudioSize);

				printf("44444");
            		}
            		if(i != num_begin && i != 0)
            		{
                		sen[i].beginTime = begin_time[i-1]*per_ms_frames;
                		sen[i].endTime = (end_time[i]+1)*per_ms_frames;
				sen[i].sentenceAudioSize = ((end_time[i]+1) - begin_time[i-1] ) * samples;
				for(int j=(begin_time[i-1]*samples);j<(end_time[i]+1)*samples;j++){
                                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
                                }


		//              sentenceVec.push_back(sen[i]);
				printf("begin_Time%d\n",sen[i].beginTime);
                                printf("end_Time%d\n",sen[i].endTime);
                                printf("数目%d\n",sen[i].sentenceAudioSize);

				printf("55555");
            		}
        	}	


        	if(i < num_end && num_begin == (num_end - 1) && begin_time[0] > end_time[0])//情况4
        	{
            		printf("情况4");
            		if(i == 0)
            		{
                		sen[i].beginTime = -1;
                		sen[i].endTime = (end_time[i]+1)*per_ms_frames;
				sen[i].sentenceAudioSize = (end_time[i]+1) * samples;

				for(int j=0;j<(end_time[i]+1)*samples;j++){
        	                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
	                        }

		//              sentenceVec.push_back(sen[i]);
                	        printf("begin_Time%d\n",sen[i].beginTime);
        	                printf("end_Time%d\n",sen[i].endTime);
	                        printf("数目%d\n",sen[i].sentenceAudioSize);


            		}
            		else
            		{
               			sen[i].beginTime = begin_time[i-1]*per_ms_frames;
               			sen[i].endTime = (end_time[i]+1)*per_ms_frames;
				sen[i].sentenceAudioSize = ((end_time[i]+1) - begin_time[i-1] ) * samples;
				for(int j=(begin_time[i-1]*samples);j<(end_time[i]+1)*samples;j++){
        	                        sen[i].sentenceAudio[j] =(int)audio_save[j];
					printf("点%d\n",sen[i].sentenceAudio[j]);
	                        }
		//              sentenceVec.push_back(sen[i]);

                	        printf("begin_Time%d\n",sen[i].beginTime);
        	                printf("end_Time%d\n",sen[i].endTime);
	                        printf("数目%d\n",sen[i].sentenceAudioSize);


            		}
        	}


        	if(i<num_begin && num_begin == 1 && num_end == 0)//情况5
        	{
               		printf("情况5");
               		sen[i].beginTime = begin_time[i]*per_ms_frames;
			sen[i].sentenceAudioSize = samplesCount - (begin_time[i] * samples);
               		sen[i].endTime = -1;
			for(int j=(begin_time[i]*samples);j<samplesCount;j++){
                                sen[i].sentenceAudio[j] =(int)audio_save[j];
				printf("点%d\n",sen[i].sentenceAudio[j]);
                        }


	//              sentenceVec.push_back(sen[i]);
                        printf("begin_Time%d\n",sen[i].beginTime);
                        printf("end_Time%d\n",sen[i].endTime);
                        printf("数目%d\n",sen[i].sentenceAudioSize);


        	}

        	if(i<num_end && num_end == 1 && num_begin == 0)//情况6
        	{
               		printf("情况6");
               		sen[i].beginTime = -1;
               		sen[i].endTime = (end_time[i]+1)*per_ms_frames;
			sen[i].sentenceAudioSize = (end_time[i]+1) * samples;

			for(int j=0;j<(end_time[i]+1) * samples;j++){
	                        sen[i].sentenceAudio[j] =(int)audio_save[j];
				printf("点%d\n",sen[i].sentenceAudio[j]);
                        }

	//              sentenceVec.push_back(sen[i]);
                        printf("begin_Time%d\n",sen[i].beginTime);
                        printf("end_Time%d\n",sen[i].endTime);
                        printf("数目%d\n",sen[i].sentenceAudioSize);
	//		printf("点%d\n",sen[i].sentenceAudio[0]);


        	}
        	printf("\n");
    	}
    return 1;

}
 
int main(int argc, char *argv[]){
	WavData song;
	ofstream out("city_16k.txt");
//	const char* fname="10.wav"; 
//	loadWavFile(fname,&song);
//	cout<<song.size<<endl;
	if (argc < 2)
	        return -1;
    	char *in_file = argv[1];
	const char* fname=(char *)in_file;

	loadWavFile(fname,&song);
	cout<<song.size<<endl;

	sentenceVec vec_sen;

	//per_ms_frames,我这里直接指定20ms一帧，vad_mode是vad的激活可选0,1,2,3 
 	vadProcess(song.data,16000,song.size,vec_sen);


	for(long i=0;i<song.size;i++){
		out<<song.data[i]<<'\n';
	}


	cout<<"end"<<endl;

	out.close();	
	freeSource(&song);
	return 0;
}

