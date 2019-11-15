#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include<vector>
#include "vad.h"

#ifndef nullptr
#define nullptr 0
#endif

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
 
int main(int argc, char *argv[]){
	WavData song;
	ofstream out("city_16k.txt");
	if (argc < 2)
	        return -1;
    	char *in_file = argv[1];
	const char* fname=(char *)in_file;
	loadWavFile(fname,&song);
	
	vector<Sentence> sentenceVec;
	vector<Sentence>* SentenceInfoVec = &sentenceVec;

	for(long i=0;i<song.size;i++){
		out<<song.data[i]<<'\n';
	}
	char* data_audio1 = (char*)song.data;

        unsigned int len = 287000;
        //unsigned int len = 32000;
        //unsigned int len = 8000;
        char audio[len*2+12];
        //char audio[len];
        //memset(audio, 0 ,sizeof(audio));
        FILE* file = fopen("tts.pcm", "rb");
        fread(audio, 1, len, file);
        fclose(file);
        file = fopen("tts.pcm", "rb");
        fread(audio+len+12, 1, len, file);
        fclose(file);
        struct timeb t1;
  struct timeb t2;

  ftime(&t1);
	//double startTime = now();
	vadPro(8000, audio, sizeof(audio), SentenceInfoVec);
ftime(&t2);

	//double time_interval = calcElapsed(startTime, now());
	//printf("time interval: %d ms\n ", (int) (time_interval * 1000));
	printf("time interval: %d ms\n ",  ((t2.time*1000+t2.millitm)-(t1.time*1000+t1.millitm)));
	size_t size = SentenceInfoVec->size();
        char ch[20];
        memset(ch, 0, sizeof(ch));
	for (size_t i = 0; i < size; i++)
        {
		cout <<"开始时间" << SentenceInfoVec->at(i).beginTime <<endl;
		cout <<"结束时间" << SentenceInfoVec->at(i).endTime <<endl;
		cout << SentenceInfoVec->at(i).senAudioSize << endl;
                sprintf(ch, "aa%d.pcm", i);
                cout<< ch <<endl; 
                FILE* file = fopen(ch, "wb");
                //char* data = (*SentenceInfoVec)[i].sentenceAudio;

                fwrite(audio+(*SentenceInfoVec)[i].senBeginInx, 1, (*SentenceInfoVec)[i].senAudioSize, file);
                fclose(file);
                memset(ch, 0, sizeof(ch));
                
        }
	


	out.close();	
	freeSource(&song);
	return 0;
}

