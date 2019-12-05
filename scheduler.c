#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROW 50
#define TIME_QUANTUM 4

int task_count=0;           //task_count here is the total num of tasks
int order[MAX_ROW];

typedef struct Taskspec{
    char name_[8];
    int arrival_;
    int burst_;
    int left_;
}Task;

void FSFC(Task tasks[MAX_ROW]);
void RR(Task tasks[MAX_ROW]);
void NSJF(Task tasks[MAX_ROW]);
void PSJF(Task tasks[MAX_ROW]);

int main(){
    FILE *TaskSpec=fopen("TaskSpec.txt","r");

    char line[256];
    Task tasks[MAX_ROW];

    while(fgets(line,sizeof(line),TaskSpec)){
        char* temp=strtok(line,",");
        strcpy(tasks[task_count].name_,temp);
        tasks[task_count].arrival_=atoi(strtok(NULL,","));
        tasks[task_count].burst_=atoi(strtok(NULL,","));
        tasks[task_count].left_=tasks[task_count].burst_;
        task_count++;
    }
    fclose(TaskSpec);

    //Make the task list in arrival time order
    int i;
    for(i=0; i<task_count;i++){
        int j;
        for(j=task_count-1;j>i;j--){
            if(tasks[j].arrival_<tasks[j-1].arrival_){
                Task temp;
                temp=tasks[j];
                tasks[j]=tasks[j-1];
                tasks[j-1]=temp;
            }
        }
    }
    FSFC(tasks);
    RR(tasks);
    NSJF(tasks);
    PSJF(tasks);
}

//First Come First Served
void FSFC(Task tasks[MAX_ROW]){
    FILE * Output=fopen("Output.txt","w");
    fprintf(Output,"FSFC:\n");
    int total_wait=0;
    int current_time=0;
    int i;
    for(i=0; i<task_count;i++){
        if(current_time<tasks[i].arrival_)     //task hasn't arrive yet
            current_time=tasks[i].arrival_;
        else                                   //task already arrived
            total_wait+=current_time-tasks[i].arrival_;
        fprintf(Output,"%s  %d  %d\n",tasks[i].name_,current_time,current_time+tasks[i].burst_);
        current_time+=tasks[i].burst_;
    }
    fprintf(Output,"Average Waiting Time: %.2f \n\n",(float)total_wait/task_count);
    fclose(Output);
}

//Round Robin
void RR(Task tasks[MAX_ROW]){
    FILE * Output=fopen("Output.txt","a");
    fprintf(Output,"RR:\n");
    int total_wait=0;
    int current_time=tasks[0].arrival_;
    int finish_count=0;

    while(finish_count!=task_count){                //All tasks has not finished
        int i;
        for(i=0; i<task_count;i++){
            if(tasks[i].arrival_>current_time)      //Start Over if not arrived
                break;

            if(tasks[i].left_ <= TIME_QUANTUM){     //Task finished
                if(tasks[i].left_==0)
                    continue;
                fprintf(Output,"%s  %d  %d\n",tasks[i].name_,current_time,current_time+tasks[i].left_);
                current_time+=tasks[i].left_;
                tasks[i].left_=0;
                finish_count++;
                total_wait+=current_time-tasks[i].arrival_-tasks[i].burst_;
            }
            else{                                   //Task unfinished
                fprintf(Output,"%s  %d  %d\n",tasks[i].name_,current_time,current_time+TIME_QUANTUM);
                tasks[i].left_-=TIME_QUANTUM;
                current_time+=TIME_QUANTUM;
            }
        }
        //If all tasks in range has finished
        if(finish_count==i)
            current_time=tasks[i].arrival_;
    }
    int k;
    for(k=0;k<task_count;k++){tasks[k].left_=tasks[k].burst_;}      //Recover the time left of each task
    fprintf(Output,"Average Waiting Time: %.2f \n\n",(float)total_wait/task_count);
    fclose(Output);
}

//Non-preemptive Shortest Job First
void NSJF(Task tasks[MAX_ROW]){
    FILE * Output=fopen("Output.txt","a");
    fprintf(Output,"NSJF:\n");
    int total_wait=0;
    int finish_count=0;
    int current_time=tasks[0].arrival_;
    
    while(finish_count!=task_count){
        int max;
        int shortest_index=-1;
        int shortest_burst=0;
        for(max=0;max<task_count && tasks[max].arrival_<=current_time;max++);//Range of arrived tasks 0 to max-1
        //Find the shortest task
        int j;
        for(j=0;j<max;j++){
            if(tasks[j].left_==0)               //Finished task
                continue;
            if(shortest_burst==0){              //Initialize the shortest task
                shortest_index=j;
                shortest_burst=tasks[j].burst_;
            }
            else if(tasks[j].burst_<shortest_burst){
                shortest_index=j;
                shortest_burst=tasks[j].burst_;
            }
        }
        //All tasks in range finished
        if(shortest_burst==0)                   
            current_time=tasks[max].arrival_;

        //Task into CPU    
        else{                                   
            finish_count++;
            fprintf(Output,"%s  %d  %d\n",tasks[shortest_index].name_,current_time,current_time+shortest_burst);
            total_wait+=current_time-tasks[shortest_index].arrival_;
            current_time+=shortest_burst;
            tasks[shortest_index].left_=0;
        }
    }
    int i;
    for(i=0;i<task_count;i++){tasks[i].left_=tasks[i].burst_;}              //Recover the time left of each task
    fprintf(Output,"Average Waiting Time: %.2f \n\n",(float)total_wait/task_count);
    fclose(Output);
}

//Preemptive Shortest Job First
void PSJF(Task tasks[MAX_ROW]){
    FILE * Output=fopen("Output.txt","a");
    fprintf(Output,"PSJF:\n");
    int total_wait=0;
    int finish_count=0;
    int current_time=tasks[0].arrival_;
    
    while(finish_count!=task_count){
        int max;
        int shortest_index=-1;
        int shortest_burst=0;
        for(max=0;max<task_count && tasks[max].arrival_<=current_time;max++);   //Range of arrived tasks 0 to max-1
        //Find the shortest task
        int j;
        for(j=0;j<max;j++){                  
            if(tasks[j].left_==0)               //Finished task
                continue;
            if(shortest_burst==0){              //Initialize the shortest task
                shortest_index=j;
                shortest_burst=tasks[j].left_;
            }
            else if(tasks[j].burst_<shortest_burst){
                shortest_index=j;
                shortest_burst=tasks[j].left_;
            }
        }
        //All tasks in range finished
        if(shortest_burst==0)                   
            current_time=tasks[max].arrival_;

        //Task into CPU
        else{                                   
            //find the next shortest burst that has not arrived yet
            int temp_index;
            for(temp_index=max;temp_index<task_count && tasks[temp_index].burst_>=tasks[shortest_index].left_;temp_index++);
            //Current task is the shortest in all task
            if(temp_index==task_count){                     
                finish_count++;
                fprintf(Output,"%s  %d  %d\n",tasks[shortest_index].name_,current_time,current_time+shortest_burst);
                total_wait+=current_time+shortest_burst-tasks[shortest_index].burst_-tasks[shortest_index].arrival_;
                current_time+=shortest_burst;
                tasks[shortest_index].left_=0;
            }
            //Fully finish
            else if(tasks[temp_index].arrival_-current_time>=tasks[shortest_index].left_){
                fprintf(Output,"%s  %d  %d\n",tasks[shortest_index].name_,current_time,current_time+tasks[shortest_index].left_);
                total_wait+=current_time+tasks[shortest_index].left_-tasks[shortest_index].arrival_-tasks[shortest_index].burst_;
                current_time+=tasks[shortest_index].left_;
                tasks[shortest_index].left_=0;
                finish_count++;
            }
            //Partially finish
            else{                                           
                fprintf(Output,"%s  %d  %d\n",tasks[shortest_index].name_,current_time,tasks[temp_index].arrival_);
                tasks[shortest_index].left_-=tasks[temp_index].arrival_-current_time;
                current_time=tasks[temp_index].arrival_;
            }
        }
    }
    fprintf(Output,"Average Waiting Time: %.2f \n\n",(float)total_wait/task_count);
    fclose(Output);
}