#define _CRT_SECURE_NO_WARNINGS //���Թ��ڲ���ȫ�����ľ���
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
#include <time.h>
using namespace std;

const int N = 1001; //���豾��ֻ����1000 pidֻ��������1~1001
int pCnt;
bool view = false;
bool  pidr[N]; //�������pid�����ظ�����

#define FORB(i,l,r) for(int i=l;i<=r;i++)

// PCB(���̿��ƿ�)
typedef struct ProcessControlBlock {
    int pid; // ����id
    int reachTime; // ���̵���ʱ��
    int execTime; // ����ִ��ʱ��
    int remainingTime; // ʣ��ִ��ʱ��
    int queuePriority; // �������ڶ���
    struct ProcessControlBlock* nextPCB; // ��ʽ�洢 ָ����һ��PCB

    void print()
    {
        printf("����id��%d������ʱ�䡾%d�� ִ��ʱ�䡾%d�� ʣ��ʱ�䡾%d�� ����%d������\n", pid, reachTime, execTime, remainingTime, queuePriority);
    }

} PCB;
PCB pcb[N];
PCB pcbf[N];//����

// MFQ(�༶��������)
typedef struct MultilevelFeedbackQueue
{
    PCB* head; // ��ͷ�ڵ�
    PCB* tail; // ��β�ڵ�
    int qSize;//��ǰ���еĴ�С
    bool isEmpty() 
    {
        return head == NULL;
    }
    int size()
    {
        return qSize;
    }
    // ��� ��βָ���ƶ�
    void enQueue(PCB* pcb) 
    {
        // �ս����ĵ�һ��PCB��Ϊ�׽ڵ�
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
    // ����
    PCB* deQueue()
    {
        // �ն��� ����NULL
        if (head == NULL)
            return NULL;
        else 
        {
            // ���Ӳ���
            PCB* temp = head;
            head = head->nextPCB;
            qSize--;
            return temp;
        }
    }
    // ��ӡ�����е�PCB
    void print() 
    {
        PCB* p = head;
        while (p != NULL) 
        {
            printf("����id��%d������ʱ�䡾%d�� ִ��ʱ�䡾%d�� ʣ��ʱ�䡾%d�� ���ڶ��С�%d��\n", p->pid, p->reachTime, p->execTime, p->remainingTime, p->queuePriority);
            p = p->nextPCB;
        }
    }

} MFQ;
typedef struct BasedOnMFQOfPMSystem
{
    MFQ* mfq[3]; // ������������
    int TimeSlice[3]; // ʱ��Ƭ
    void init() 
    {
        FORB(i,0,2)
        {
            mfq[i] = (MFQ*)malloc(sizeof(MFQ));
            mfq[i]->head = mfq[i]->tail = NULL;
            mfq[i]->qSize = 0;
            TimeSlice[i] = 1<<i ; // ʱ��Ƭ���� 1 2 4
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
            printf("������ӽ�����~ \n\n");
            system("pause");
            return;
        }
        //����
        sort(pcb, pcb + pCnt, [&](const PCB x, const PCB y) {
            if (x.reachTime == y.reachTime)
                return x.execTime < y.execTime;
            return x.reachTime < y.reachTime;
            });
        int currentTime = 0; //��ǰʱ��
        int completedProcesses = 0; //����ɵĽ���
        int index = 0;
        //�ѵ���ʱ��Ϊ0��ɨ��һ��
        FORB(i, 0, pCnt-1)
            if (pcb[i].reachTime == currentTime)
            {
                mfq[0]->enQueue(&pcb[i]);
                printf("\n��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[i].pid);
                index = i + 1;
            }
            else break;
        if (mfq[0]->isEmpty()) //���û�н��̵���ʱ����0 �򵽴�ʱ������Ľ������ȷ��������
        {
            FORB(i, 0, pcb[0].reachTime - 1)
            {
                if (view&&i)
                {
                    printf("\n");
                    system("pause");
                    system("cls");
                }
                printf("\n��ǰʱ�䣨%d���� ������δɨ�赽�κν��̽��롭�� \n", i, pcb[index - 1].pid);
            }
            currentTime = pcb[0].reachTime;
               
            while (pcb[index].reachTime == currentTime)
            {
                mfq[0]->enQueue(&pcb[index++]);
                printf("\n��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[index - 1].pid);
            }    
        }
        while (completedProcesses < pCnt) //�����н���δִ���������ִ��
        {
            FORB(i,0,2)
            {
                cat:
                while (!mfq[i]->isEmpty()) //��ǰ���л��н���ʱ����ִ��
                {
                    PCB *currentProcess = mfq[i]->deQueue();

                    if (currentProcess->remainingTime > TimeSlice[i]) 
                    {
                        //�ж� 1 2 �ڶ������������������
                        if (i)
                        {
                            //�ڶ�������������������еĽ���ÿ�붼���ж�һ���Ƿ����µĽ��̽���
                            FORB(time, 1, TimeSlice[i])
                            {
                                bool sign = false;
                                bool ifmove = mfq[0]->isEmpty();//�����жϸտ�ʼ1������Ϊ�� ��һ���ͻȻ����>=1�����̵����
                                int move = 0;
                                //�жϵ�ǰʱ����Ƿ����½��̽���
                                while (pcb[index].reachTime == currentTime)
                                {
                                    sign = true;
                                    move++;
                                    mfq[0]->enQueue(&pcb[index++]);
                                    printf("��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[index - 1].pid);
                                }
                                if (sign)//�����1�������н��� ������� ��Ϊ�������ֲ�����ִ�С�
                                {
                                    if (ifmove&&move>=1) //�������1ֻ��һ������ �������ֹ��Ϣ 
                                    {
                                        PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                        if (cur == NULL) //��ǰ�����Ƕ����е����һ������ ȡ������պñ���ˡ�
                                        {
                                            printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                        }
                                        else {
                                            mfq[currentProcess->queuePriority]->enQueue(cur);
                                            printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                        }
                                    }
                                    i = 0; //��������1������ִ��
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
                                printf("\n��ǰʱ�䣨%d���� ���̡�%d����ִ��%d�� ʣ��ִ��ʱ��%d��\n", currentTime, currentProcess->pid, 1, currentProcess->remainingTime);

                            }//for
                        }//�ڶ����if
                        else 
                        {
                            //�жϵ�ǰʱ����Ƿ����½��̽���
                            while (pcb[index].reachTime == currentTime)
                            {
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[index - 1].pid);
                            }
                            // ʹ��ʱ���һ������Ƭ
                            currentProcess->remainingTime -= TimeSlice[i];
                            currentTime += TimeSlice[i];
                            if (view)
                            {
                                printf("\n");
                                system("pause");
                                system("cls");
                            }
                            printf("\n��ǰʱ�䣨%d���� ���̡�%d����ִ��%d�� ʣ��ִ��ʱ��%d��\n", currentTime, currentProcess->pid, TimeSlice[i], currentProcess->remainingTime);
                        }
                        //����һ�������м���������
                        if (i < 2)
                        {
                            currentProcess->queuePriority = i + 1;
                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                            printf("��ǰʱ�䣨%d���� ʱ��Ƭ��%ds�������� �����̡�%d����δִ�н��� ����%d�������С���\n", currentTime,TimeSlice[i], currentProcess->pid, currentProcess->queuePriority+1);
                            
                            bool sign = false;
                            bool ifmove = mfq[0]->isEmpty();//�����жϸտ�ʼ1������Ϊ�� ��һ���ͻȻ����>=1�����̵����
                            int move = 0;
                            //�жϵ�ǰʱ����Ƿ����½��̽���
                            while (pcb[index].reachTime == currentTime)
                            {
                                sign = true;
                                move++;
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[index - 1].pid);
                            }
                            if (sign)//�����1�������н��� ������� ��Ϊ�������ֲ�����ִ�С�
                            {
                                if (ifmove && move >= 1) //�������1ֻ��һ������ �������ֹ��Ϣ 
                                {
                                    PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                    if (cur == NULL) //��ǰ�����Ƕ����е����һ������ ȡ������պñ���ˡ�
                                    {
                                        printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                        mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                    }
                                    else {
                                        mfq[currentProcess->queuePriority]->enQueue(cur);
                                        printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                    }
                                }
                                i = 0; //��������1������ִ��
                                goto cat;
                            }
                        }
                        else 
                        {
                            // �����������ȼ������л�δ��ɣ������¼������
                            currentProcess->queuePriority = 2;
                            mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                            printf("��ǰʱ�䣨%d���� ʱ��Ƭ��%ds�������� �����̡�%d����δִ�н��� ����%d������ĩβ��~����\n", currentTime, TimeSlice[i],currentProcess->pid, currentProcess->queuePriority + 1);

                            bool sign = false;
                            bool ifmove = mfq[0]->isEmpty();//�����жϸտ�ʼ1������Ϊ�� ��һ���ͻȻ����>=1�����̵����
                            int move = 0;
                            //�жϵ�ǰʱ����Ƿ����½��̽���
                            while (pcb[index].reachTime == currentTime)
                            {
                                sign = true;
                                move++;
                                mfq[0]->enQueue(&pcb[index++]);
                                printf("��ǰʱ�䣨%d���� ���棡���棡 ���̡�%d������1�����У��� \n", currentTime, pcb[index - 1].pid);
                            }
                            if (sign)//�����1�������н��� ������� ��Ϊ�������ֲ�����ִ�С�
                            {
                                if (ifmove && move >= 1) //�������1ֻ��һ������ �������ֹ��Ϣ 
                                {
                                    PCB* cur = mfq[currentProcess->queuePriority]->deQueue();
                                    if (cur == NULL) //��ǰ�����Ƕ����е����һ������ ȡ������պñ���ˡ�
                                    {
                                        printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, currentProcess->pid, currentProcess->queuePriority + 1);
                                        mfq[currentProcess->queuePriority]->enQueue(currentProcess);
                                    }
                                    else {
                                        mfq[currentProcess->queuePriority]->enQueue(cur);
                                        printf("��ǰʱ�䣨%d���� ���̡�%d��ʶȤ��ִֹͣ�� ��ص���ǰ%d������ĩβ�����ȴ���~\n", currentTime, cur->pid, cur->queuePriority + 1);
                                    }
                                }
                                i = 0; //��������1������ִ��
                                goto cat;
                            }
                        }
                    }//������if
                    else 
                    {
                        currentTime += currentProcess->remainingTime;
                        if (view)
                        {
                            printf("\n");
                            system("pause");
                            system("cls");
                        }
                        printf("\n��ǰʱ�䣨%d���� ���̡�%d����ִ��%d�� ʣ��ִ��ʱ��%d�� ִ����ϣ��ѱ��ڴ����ͷ���~\n", currentTime, currentProcess->pid, currentProcess->remainingTime, 0);
                        currentProcess->remainingTime = 0;
                        completedProcesses++;
                    }
                }
            }
        }
        //״̬�ָ�
        FORB(i, 0, pCnt - 1)
            pcb[i] = pcbf[i];

        printf("\n");
        system("pause");
        system("cls");
        printf("\a");
        printf("������ʾ�͵���������� ��л���Ĺۿ�~\n\n");
        system("pause");
    }
} MFQS;

void add()
{
    system("cls");
    printf("\a");
    printf("����������Ҫ�����Ľ��̸����� ");

    int n;
    scanf_s("%d", &n);
    int index = pcb[0].pid ? pCnt : 0; //��ֹ����
    pCnt += n;

    FORB(i, index, pCnt - 1)
    {
        system("cls");
        printf("\a");
        printf("�� ��ܰС��ʾ ��\n\n");
        printf("��1�������ʽ������id ����ʱ�� ִ��ʱ��\n\n");
        printf("��2���Ϸ�����id��Χ(1~1000��Χ��)\n\n ");
        printf("�����룺");

        scanf_s("%d %d %d", &pcb[i].pid, &pcb[i].reachTime, &pcb[i].execTime);
        if (pcb[i].pid < 1 || pcb[i].pid>1000)
        {
            system("cls");
            printf("\a");
            printf("�����pid��ֵ��Χ����(����1~1000��Χ��)������������~");
            system("pause");
            i--;
            continue;
        }
        if (pidr[pcb[i].pid])
        {
            system("cls");
            printf("\a");
            printf("ϵͳ�����д�pid��~��������������~\n\n");
            system("pause");
            i--;
            continue;
        }
        else pidr[pcb[i].pid] = true;

        pcb[i].remainingTime = pcb[i].execTime;
        pcb[i].queuePriority = 0; //�տ�ʼ������뵽���ȼ���ߵĶ���(0)����

        pcbf[i] = pcb[i];
    }
    system("cls");
    printf("\a");
    printf("��ӳɹ�! ���Ե�~");
    Sleep(2000);
}
void del()
{
    system("cls");
    printf("\a");
    printf("����������Ҫɾ���Ľ���pid(����0��ɾ������)�� ");
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
                printf("ɾ���ɹ�! ���Ե�~");
                Sleep(2000);

                pCnt -= 1;
                return;
            }
        }
        system("cls");
        printf("\a");
        printf("ɾ��ʧ��! ϵͳ�����Ҳ����˽��̵�pid��~");
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
        printf("ȫ����ɾ�����~");
        Sleep(2000);
    }
}
void select()
{
    //����
    sort(pcb, pcb + pCnt, [&](const PCB x, const PCB y) {
        if (x.reachTime == y.reachTime)
            return x.execTime < y.execTime;
        return x.reachTime < y.reachTime;
        });
    system("cls");
    printf("\a");
    printf("��������Ҫ��ѯ���̵�pid(����0�ɲ�ѯ����)�� ");

    int pid;
    scanf_s("%d", &pid);
    if (pid)
    {
        FORB(i,0,pCnt-1)
            if (pcb[i].pid == pid)
            {
                system("cls");
                printf("��ѯ�ɹ���\n\n");
                Sleep(800);
                system("cls");
                pcb[i].print();
                printf("\n");
                system("pause");
                return;
            }
        system("cls");
        printf("\a");
        printf("��ѯʧ�ܣ� �ƺ�û�иý��̽��뱾ϵͳ��~\n\n");
        Sleep(1500);
        return;
    }
    else {
        system("cls");
        printf("\a");
        printf("����ǰλ��ϵͳ�еĽ��� %d���� \n\n",pCnt);
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

    printf("����������Ҫ���µĽ���pid�� ");
    scanf_s("%d", &pid);
    printf("\a");

    FORB(i, 0, pCnt - 1)
    {
        if (pcb[i].pid == pid)
        {
            printf("����Ľ������԰ɡ�����ʱ�� ִ��ʱ�䡿\n\n");
            scanf_s("%d %d",&pcb[i].reachTime,&pcb[i].execTime);
            system("cls");
            printf("\a");
            printf("���³ɹ�! ���Ե�~");
            Sleep(2000);

            pcbf[i] = pcb[i];
            return;
        }
    }
    system("cls");
    printf("\a");
    printf("����ʧ��! ϵͳ�����Ҳ����˽��̵�pid��~");
    Sleep(2000);
}

void view3()
{
    MFQS os;
    os.init();//��ʼ������ ����ʱ��Ƭ
first:
    system("color 0c");
    system("cls");
    printf("���ܼ���1�� ���ż�ɽ~ (ֱ�Ӳ鿴���н��̵ĵ��ȹ���)\n");
    printf("���ܼ���2�� ��������! (������˵ģʽ���ۿ����ȹ���)\n");
    printf("���ܼ���3�� ��ӽ���\n");
    printf("���ܼ���4�� ɾ������\n");
    printf("���ܼ���5�� ���ҽ���\n");
    printf("���ܼ���6�� ���Ľ���\n");
    printf("���ܼ���7�� ������ҳ\n\n");
    printf("����������ѡ��(������Ӧ���ܼ�����ѡ������Ҫ�ι���~) : ");

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
        printf("���ڷ���ϵͳ�� ��ǰ���н��̽��ᱻ�������");
        os.destory();
        Sleep(3000);
        return;
    }
    else {
        printf("\a");
        system("cls");
        printf("�������3��󷵻���ҳ����");
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
        printf("ϵͳ����������(%d)��������\a", n--);
        Sleep(1000);
    }
    system("cls");
}
void view1()
{
    start:
    system("cls");
    system("color 0c");
    printf("��ӭ���ٱ�ϵͳ~\n\n����������ѡ��(������Ӧ���ּ�ѡ������Ҫ�ι���~) \n\n");
    printf("���ּ�1:������ϵͳ~\n\n");
    printf("���ּ�2:�˳���ϵͳ��\n\n  \t ");

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
        printf("��лʹ�ã���ӭ�´ι�����~");
        exit(0);
    } else {
        system("cls");
        printf("\n���������������������롭��");
        Sleep(1000);
        goto start;
    }
}
void start()
{
    printf("����������������������������������������������������������������\n\n");
    printf("�����ڶ༶�������еĽ��̹���ϵͳ��\n\n");

    //��ȡ��ǰʱ�䣨��1970��1��1��00:00:00�����ڵ�������
    time_t nowTime = time(NULL);
    // ��ʱ��ת��Ϊ����ʱ��
    struct tm* local_time = localtime(&nowTime);

    printf("����ǰ���ڣ�%d��%02d��%02d�ա�\n\n",
        local_time->tm_year + 1900, 
        local_time->tm_mon + 1,     
        local_time->tm_mday);       
    printf("����ǰʱ�䣺%02d:%02d:%02d��\n\n",
        local_time->tm_hour,       
        local_time->tm_min,        
        local_time->tm_sec);       
    printf("����������������������������������������������������������������\n\n");
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
//    ��������
// 
//    ��������һ
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
//    ����������
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
//    ����������
//    3
//    1 0 8
//    2 1 4
//    3 5 1
// */