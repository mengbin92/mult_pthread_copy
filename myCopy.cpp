#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>

using namespace std;

const int maxThread = 5;

struct Info
{
    char* fromFile;     //源文件
    char* toFile;       //目标文件
    int num;            //第几个线程
};

bool isFile(char* path)
{
    struct stat buf;
    stat(path,&buf);
    if(S_ISREG(buf.st_mode))
        return true;
    else
        return false;
}

unsigned long int getSize(char* fileName)
{
    struct stat buf;
    stat(fileName,&buf);
    return buf.st_size;
}

void* doThread(void *arg)
{
    Info* info = (Info*)arg;

    unsigned long int per = getSize(info->fromFile)/maxThread;

    FILE* fin = fopen(info->fromFile,"r");
    FILE* fout = fopen(info->toFile,"w+");

    fseek(fin,info->num*per,SEEK_SET);
    fseek(fout,info->num*per,SEEK_SET);

    char buf[4096] = {0};
    int n;
    int sum = 0;
    while((n = fread(buf,1,sizeof(buf),fin)) > 0)
    {
        fwrite(buf,1,n,fout);
        if(info->num == (maxThread-1))
            cout<<"sum = "<<sum<<" per = "<<per<<endl;
        sum += n;
        if(sum > per)
            break;
        memset(buf,0,sizeof(buf));
    }

    fclose(fin);
    fclose(fout);

    return NULL;
}

int main(int argc,char* argv[])
{
#if 1
    if(argc != 3)
    {
        cout<<"Usage: ./myCopy srcFile dstFile"<<endl;
        return -1;
    }
#endif

    if(!isFile(argv[1]))
    {
        cout<<argv[1]<<"is not a regular file !!!"<<endl;
        return -1;
    }

    struct timeval start;
    struct timeval end;
    double diff;

    Info info[maxThread];

    FILE* fout = fopen(argv[2],"w");
    fclose(fout);
    truncate(argv[2],getSize(argv[1]));

    pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t)*maxThread);
    if(tid == NULL)
    {
        cout<<"Malloc Error"<<endl;
        return -1;
    }
    //start
    gettimeofday(&start,NULL);

    for(int i = 0; i < maxThread; ++i)
    {
        info[i].fromFile = argv[1];
        info[i].toFile = argv[2];
        info[i].num = i;

        pthread_create(&tid[i],NULL,doThread,(void*)&info[i]);
    }

    for(int i = 0; i< maxThread;++i)
        pthread_join(tid[i],NULL);
    free(tid);


    //end
    gettimeofday(&end,NULL);
    diff = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
    cout<<diff/1000000<<"s"<<endl;
    return 0;
}
