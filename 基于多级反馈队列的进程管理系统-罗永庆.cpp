#define _CRT_SECURE_NO_WARNINGS //忽略关于不安全函数的警告
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
#include <time.h>
using namespace std;

const int N = 1001; //假设本喵只接收1000 pid只允许设置1~1001
int pCnt;
bool view = false;
bool  pidr[N]; //解决进程pid不可重复问题

#define FORB(i,l,r) for(int i=l;i<=r;i++)

// PCB(进程控制块)
typedef struct ProcessControlBlock {
    int pid; // 进程id
    int reachTime; // 进程到达时间
    int execTime; // 进程执行时间
    int remainingTime; // 剩余执行时间
    int queuePriority; // 进程所在队列
    struct ProcessControlBlock* nextPCB; // 链式存储 指向下一个PCB

    void print()
    {
        printf("进程id【%d】到达时间【%d】 执行时间【%d】 剩余时间【%d】 处于%d级队列\n", pid, reachTime, execTime, remainingTime, queuePriority);
    }

} PCB;
PCB pcb[N];
PCB pcbf[N];//副本

// MFQ(多级反馈队列)
typedef struct MultilevelFeedbackQueue
{
    PCB* head; // 队头节点
    PCB* tail; // 队尾节点
    int qSize;//当前队列的大小
    bool isEmpty() 
    {
        return head == NULL;
    }
    int size()
    {
        return qSize;
    }
    // 入队 队尾指针移动
    void enQueue(PCB* pcb) 
    {
        // 刚进来的第一个PCB做为首节点
        if (head == NULL)
            head = tail = pcb;
        else 
        {
            tail->nextPCB = pcb;
            tail = pcb;
            tail->nextPCB = NULL;
        }
        qSize++;
    }
    // 出队
    PCB* deQueue()
    {
        // 空队列 返回NULL
        if (head == NULL)
            return NULL;
        else 
        {
            // 出队操作
            PCB* temp = head;
            head = head->nextPCB;
            qSize--;
            return temp;
        }
    }
    // 打印队列中的PCB
    void print() 
    {
        PCB* p = head;
        while (p != NULL) 
        {
            printf("进程id【%d】到达时间【%d】 执行时间【%d】 剩余时间【%d】 所在队列【%d】\n", p->pid, p->reachTime, p->execTime, p->remainingTime, p->queuePriority);
            p = p->nextPCB;
        }
    }

} MFQ;
typedef struct BasedOnMFQOfPMSystem
{
    MFQ* mfq[3]; // 三级反馈队列
    int TimeSlice[3]; // 时间片
    void init() 
    {
        FORB(i,0,2)
        {
            mfq[i] = (MFQ*)malloc(sizeof(MFQ));
            mfq[i]->head = mfq[i]->tail = NULL;
            mfq[i]->qSize = 0;
            TimeSlice[i] = 1<<i ; // 时间片分配 1 2 4
        }
    }
    void destory()
    {
        FORB(i, 0, 2)
            delete mfq[i];
        FORB(i, 0, N - 1)
            pidr[pcb[i].pid] = false;
        FORB(i, 0, pCnt - 1)
        {
            pcb[i].pid = 0;
            pcb[i].reachTime = 0;
            pcb[i].execTime = 0;
            pcb[i].remainingTime = 0;
            pcb[i].queuePriority = 0;
            pcb[i].nextPCB = NULL;
            pcbf[i] = pcb[i];
        }
        pCnt = 0;
        view = false;
    }

    void schedule() 
    {
        system("cls");
        if (!pCnt)
        {
            printf("请先添加进程喵~ \n\n");
            system("pause");
            return;
        }
        //升序
        sort(pcb, pcb + pCnt, [&](const PCB x, const PCB y) {
            if (x.reachTime == y.reachTime)
                return x.execTime < y.execTime;
            return x.reachTime < y.reachTime;
            });
        int currentTime = 0; //当前时间
        int completedProcesses = 0; //已完成的进程
        int index = 0;
        //把到达时间为0的扫描一遍
        FORB(i, 0, pCnt-1)
            if (pcb[i].reachTime == currentTime)
            {
                mfq[0]->enQueue(&pcb[i]);
                printf("\n当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[i].pid);
                index = i + 1;
            }
            else break;
        if (mfq[0]->isEmpty()) //如果没有进程到达时刻是0 则到达时间最早的进程优先放入队列中
        {
            FORB(i, 0, pcb[0].reachTime - 1)
            {
                if (view&&i)
                {
                    printf("\n");
                    system("pause");
                    system("cls");
                }
                printf("\n当前时间（%d）： 队列中未扫描到任何进程进入…… \n", i, pcb[index - 1].pid);
            }
            currentTime = pcb[0].reachTime;
               
            while (pcb[index].reachTime == currentTime)
            {
                mfq[0]->enQueue(&pcb[index++]);
                printf("\n当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[index - 1].pid);
            }    
        }
        while (completedProcesses < pCnt) //当还有进程未执行完则继续执行
        {
            FORB(i,0,2)
            {
                cat:
                while (!mfq[i]->isEmpty()) //当前队列还有进程时继续执行
                {
                    PCB *currentProcess = mfq[i]->deQueue();

                    if (currentProcess->remainingTime > TimeSlice[i]) 
                    {
                        //判断 1 2 第二级队列与第三级队列
                        if (i)
                        {
                            //第二级队列与第三级队列中的进程每秒都得判断一下是否有新的进程进来
                            FORB(time, 1, TimeSlice[i])
                            {
                                bool sign = false;
                                bool ifmove = mfq[0]->isEmpty();//用于判断刚开始1级队列为空 下一秒就突然来了>=1个进程的情况
                                int move = 0;
                                //判断当前时间点是否有新进程进来
                                while (pcb[index].reachTime == currentTime)
                                {
                                    sign = true;
                                    move++;
                                    mfq[0]->enQueue(&pcb[index++]);
                                    printf("当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[index - 1].pid);
                                }
                                if (sign)//如果第1级队列有进程 则不用输出 因为根本就轮不到它执行。
                                {
                                    if (ifmove&&move>=1) //如果队列1只有一个进程 则输出终止信息 
                                    {
                                        PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                        if (cur == NULL) //当前进程是队列中的最后一个进程 取出来后刚好变空了。
                                        {
                                            printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                        }
                                        else {
                                            mfq[currentProcess->queuePriority]->enQueue(cur);
                                            printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                        }
                                    }
                                    i = 0; //重新来到1级队列执行
                                    goto cat;
                                }
                                currentProcess->remainingTime--;
                                currentTime++;
                                if (view)
                                {
                                    printf("\n");
                                    system("pause");
                                    system("cls");
                                }
                                printf("\n当前时间（%d）： 进程【%d】已执行%d秒 剩余执行时间%d秒\n", currentTime, currentProcess->pid, 1, currentProcess->remainingTime);

                            }//for
                        }//第二层的if
                        else 
                        {
                            //判断当前时间点是否有新进程进来
                            while (pcb[index].reachTime == currentTime)
                            {
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[index - 1].pid);
                            }
                            // 使用时间第一级队列片
                            currentProcess->remainingTime -= TimeSlice[i];
                            currentTime += TimeSlice[i];
                            if (view)
                            {
                                printf("\n");
                                system("pause");
                                system("cls");
                            }
                            printf("\n当前时间（%d）： 进程【%d】已执行%d秒 剩余执行时间%d秒\n", currentTime, currentProcess->pid, TimeSlice[i], currentProcess->remainingTime);
                        }
                        //处在一二级队列继续往后走
                        if (i < 2)
                        {
                            currentProcess->queuePriority = i + 1;
                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                            printf("当前时间（%d）： 时间片【%ds】已用完 但进程【%d】还未执行结束 送往%d级队列中……\n", currentTime,TimeSlice[i], currentProcess->pid, currentProcess->queuePriority+1);
                            
                            bool sign = false;
                            bool ifmove = mfq[0]->isEmpty();//用于判断刚开始1级队列为空 下一秒就突然来了>=1个进程的情况
                            int move = 0;
                            //判断当前时间点是否有新进程进来
                            while (pcb[index].reachTime == currentTime)
                            {
                                sign = true;
                                move++;
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[index - 1].pid);
                            }
                            if (sign)//如果第1级队列有进程 则不用输出 因为根本就轮不到它执行。
                            {
                                if (ifmove && move >= 1) //如果队列1只有一个进程 则输出终止信息 
                                {
                                    PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                    if (cur == NULL) //当前进程是队列中的最后一个进程 取出来后刚好变空了。
                                    {
                                        printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                        mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                    }
                                    else {
                                        mfq[currentProcess->queuePriority]->enQueue(cur);
                                        printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                    }
                                }
                                i = 0; //重新来到1级队列执行
                                goto cat;
                            }
                        }
                        else 
                        {
                            // 如果在最低优先级队列中还未完成，则重新加入队列
                            currentProcess->queuePriority = 2;
                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                            printf("当前时间（%d）： 时间片【%ds】已用完 但进程【%d】还未执行结束 送往%d级队列末尾啦~……\n", currentTime, TimeSlice[i],currentProcess->pid, currentProcess->queuePriority + 1);

                            bool sign = false;
                            bool ifmove = mfq[0]->isEmpty();//用于判断刚开始1级队列为空 下一秒就突然来了>=1个进程的情况
                            int move = 0;
                            //判断当前时间点是否有新进程进来
                            while (pcb[index].reachTime == currentTime)
                            {
                                sign = true;
                                move++;
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("当前时间（%d）： 警告！警告！ 进程【%d】降临1级队列！！ \n", currentTime, pcb[index - 1].pid);
                            }
                            if (sign)//如果第1级队列有进程 则不用输出 因为根本就轮不到它执行。
                            {
                                if (ifmove && move >= 1) //如果队列1只有一个进程 则输出终止信息 
                                {
                                    PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                    if (cur == NULL) //当前进程是队列中的最后一个进程 取出来后刚好变空了。
                                    {
                                        printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                        mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                    }
                                    else {
                                        mfq[currentProcess->queuePriority]->enQueue(cur);
                                        printf("当前时间（%d）： 进程【%d】识趣的停止执行 溜回到当前%d级队列末尾继续等待喵~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                    }
                                }
                                i = 0; //重新来到1级队列执行
                                goto cat;
                            }
                        }
                    }//最外层的if
                    else 
                    {
                        currentTime += currentProcess->remainingTime;
                        if (view)
                        {
                            printf("\n");
                            system("pause");
                            system("cls");
                        }
                        printf("\n当前时间（%d）： 进程【%d】已执行%d秒 剩余执行时间%d秒 执行完毕，已被内存大大释放啦~\n", currentTime, currentProcess->pid, currentProcess->remainingTime, 0);
                        currentProcess->remainingTime = 0;
                        completedProcesses++;
                    }
                }
            }
        }
        //状态恢复
        FORB(i, 0, pCnt - 1)
            pcb[i] = pcbf[i];

        printf("\n");
        system("pause");
        system("cls");
        printf("\a");
        printf("过程演示就到此完毕啦！ 感谢您的观看~\n\n");
        system("pause");
    }
} MFQS;

void add()
{
    system("cls");
    printf("\a");
    printf("请输入您想要创建的进程个数： ");

    int n;
    scanf_s("%d", &n);
    int index = pcb[0].pid ? pCnt : 0; //防止覆盖
    pCnt += n;

    FORB(i, index, pCnt - 1)
    {
        system("cls");
        printf("\a");
        printf("《 温馨小提示 》\n\n");
        printf("【1】输入格式：进程id 到达时间 执行时间\n\n");
        printf("【2】合法进程id范围(1~1000范围内)\n\n ");
        printf("请输入：");

        scanf_s("%d %d %d", &pcb[i].pid, &pcb[i].reachTime, &pcb[i].execTime);
        if (pcb[i].pid < 1 || pcb[i].pid>1000)
        {
            system("cls");
            printf("\a");
            printf("输入的pid数值范围有误(不在1~1000范围内)，请重新输入~");
            system("pause");
            i--;
            continue;
        }
        if (pidr[pcb[i].pid])
        {
            system("cls");
            printf("\a");
            printf("系统中已有此pid啦~，请重新输入喵~\n\n");
            system("pause");
            i--;
            continue;
        }
        else pidr[pcb[i].pid] = true;

        pcb[i].remainingTime = pcb[i].execTime;
        pcb[i].queuePriority = 0; //刚开始都会进入到优先级最高的队列(0)当中

        pcbf[i] = pcb[i];
    }
    system("cls");
    printf("\a");
    printf("添加成功! 请稍等~");
    Sleep(2000);
}
void del()
{
    system("cls");
    printf("\a");
    printf("请输入您想要删除的进程pid(输入0可删除所有)： ");
    int pid;
    scanf_s("%d", &pid);
    
    if (pid)
    {
        FORB(i, 0, pCnt - 1)
        {
            if (pcb[i].pid == pid)
            {
                pidr[pcb[i].pid] = false;
                FORB(j, i, pCnt - 1)
                    pcb[j] = pcb[j + 1];
                FORB(k, 0, pCnt-1)
                    pcbf[k] = pcb[k];
                system("cls");
                printf("\a");
                printf("删除成功! 请稍等~");
                Sleep(2000);

                pCnt -= 1;
                return;
            }
        }
        system("cls");
        printf("\a");
        printf("删除失败! 系统好像找不到此进程的pid呢~");
        Sleep(2000);
    }
    else
    {
        FORB(i,0,pCnt-1)
        {
            pidr[pcb[i].pid] = false;
            pcb[i].pid = 0;
            pcb[i].reachTime = 0;
            pcb[i].execTime = 0;
            pcb[i].remainingTime = 0;
            pcb[i].queuePriority = 0;
            pcb[i].nextPCB = NULL;
            pcbf[i] = pcb[i];
        }
        pCnt = 0;

        system("cls");
        printf("\a");
        printf("全部已删光光啦~");
        Sleep(2000);
    }
}
void select()
{
    //升序
    sort(pcb, pcb + pCnt, [&](const PCB x, const PCB y) {
        if (x.reachTime == y.reachTime)
            return x.execTime < y.execTime;
        return x.reachTime < y.reachTime;
        });
    system("cls");
    printf("\a");
    printf("请输入所要查询进程的pid(输入0可查询所有)： ");

    int pid;
    scanf_s("%d", &pid);
    if (pid)
    {
        FORB(i,0,pCnt-1)
            if (pcb[i].pid == pid)
            {
                system("cls");
                printf("查询成功！\n\n");
                Sleep(800);
                system("cls");
                pcb[i].print();
                printf("\n");
                system("pause");
                return;
            }
        system("cls");
        printf("\a");
        printf("查询失败！ 似乎没有该进程进入本系统呢~\n\n");
        Sleep(1500);
        return;
    }
    else {
        system("cls");
        printf("\a");
        printf("【当前位于系统中的进程 %d个】 \n\n",pCnt);
        FORB(i, 0, pCnt - 1)
        {
            pcb[i].print();
            printf("\n\n");
        }
        system("pause");
    }
}
void update()
{
    system("cls");
    int pid;

    printf("请输入您想要更新的进程pid： ");
    scanf_s("%d", &pid);
    printf("\a");

    FORB(i, 0, pCnt - 1)
    {
        if (pcb[i].pid == pid)
        {
            printf("请更改进程属性吧【到达时间 执行时间】\n\n");
            scanf_s("%d %d",&pcb[i].reachTime,&pcb[i].execTime);
            system("cls");
            printf("\a");
            printf("更新成功! 请稍等~");
            Sleep(2000);

            pcbf[i] = pcb[i];
            return;
        }
    }
    system("cls");
    printf("\a");
    printf("更新失败! 系统好像找不到此进程的pid呢~");
    Sleep(2000);
}

void view3()
{
    MFQS os;
    os.init();//初始化队列 分配时间片
first:
    system("color 0c");
    system("cls");
    printf("功能键【1】 开门见山~ (直接查看所有进程的调度过程)\n");
    printf("功能键【2】 兵来将挡! (进入演说模式来观看调度过程)\n");
    printf("功能键【3】 添加进程\n");
    printf("功能键【4】 删除进程\n");
    printf("功能键【5】 查找进程\n");
    printf("功能键【6】 更改进程\n");
    printf("功能键【7】 返回首页\n\n");
    printf("请输入您的选择(输入相应功能键数字选择你想要滴功能~) : ");

    int choice=0;
    scanf_s("%d", &choice);
    if (choice == 1)
    {
        printf("\a");
        view = false;
        os.schedule();
        goto first;
    }
    else  if (choice == 2)
    {
        printf("\a");
        view = true;
        os.schedule();
        goto first;
    }
    else  if (choice == 3)
    {
        printf("\a");
        add();
        goto first;
    }
    else  if (choice == 4)
    {
        printf("\a");
        del();
        goto first;
    }
    else  if (choice == 5)
    {
        printf("\a");
        select();
        goto first;
    }
    else  if (choice == 6)
    {
        printf("\a");
        update();
        goto first;
    }
    else  if (choice == 7)
    {
        system("cls");
        printf("\a");
        printf("正在返回系统中 当前所有进程将会被清除……");
        os.destory();
        Sleep(3000);
        return;
    }
    else {
        printf("\a");
        system("cls");
        printf("输入错误，3秒后返回首页……");
        Sleep(3000);
    }
    system("cls");
}
void OSDesign()
{
    view3();
}
void view2()
{
    int n = 3;
    while (n)
    {
        system("cls");
        printf("系统正在启动中(%d)…………\a", n--);
        Sleep(1000);
    }
    system("cls");
}
void view1()
{
    start:
    system("cls");
    system("color 0c");
    printf("欢迎光临本系统~\n\n请输入您的选择(输入相应数字键选择你想要滴功能~) \n\n");
    printf("数字键1:启动本系统~\n\n");
    printf("数字键2:退出本系统！\n\n  \t ");

    int choice;
    scanf_s("%d", &choice);
    if (choice == 1)
    {
        view2();
        OSDesign();
        goto start;
    }
    else if(choice == 2)
    {
        system("cls");
        printf("感谢使用，欢迎下次光临喵~");
        exit(0);
    } else {
        system("cls");
        printf("\n输入有误喵，请重新输入……");
        Sleep(1000);
        goto start;
    }
}
void start()
{
    printf("————————————————————————————————\n\n");
    printf("【基于多级反馈队列的进程管理系统】\n\n");

    //获取当前时间（从1970年1月1日00:00:00到现在的秒数）
    time_t nowTime = time(NULL);
    // 将时间转换为本地时间
    struct tm* local_time = localtime(&nowTime);

    printf("【当前日期：%d年%02d月%02d日】\n\n",
        local_time->tm_year + 1900, 
        local_time->tm_mon + 1,     
        local_time->tm_mday);       
    printf("【当前时间：%02d:%02d:%02d】\n\n",
        local_time->tm_hour,       
        local_time->tm_min,        
        local_time->tm_sec);       
    printf("————————————————————————————————\n\n");
    system("pause");
    printf("\a");
    view1();
}
void mainView()
{
    start();
}
int main()
{
    mainView();
    return 0;
}
///*
//
//    测试数据
// 
//    测试用例一
//    9
//    1 0 8
//    2 1 4
//    3 2 2
//    4 2 1
//    5 3 8
//    6 3 4
//    7 4 2
//    8 4 1
//    9 5 8
//
//    测试用例二
//    9
//    1 7 8
//    2 2 4
//    3 2 2
//    4 5 1
//    5 6 8
//    6 4 4
//    7 2 2
//    8 4 1
//    9 3 8
//
//    测试用例三
//    3
//    1 0 8
//    2 1 4
//    3 5 1
// */