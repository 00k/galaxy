### console models ���

��Ҫ����һ�¼������
* service �൱�򵥾���һ�����������Ҫ��¼һ���������ƣ������ˣ�ʱ��������Ϣ
* task �������
* taskgroup �����飬һ��service����ʱ���и�Ĭ��taskgroup0�飬�������е�ʵ������С��������ʱ�ᴴ��һ��taskgroup1��ʱ�飬����С����ʵ������ȫ����ʱ�����taskgroup1ʵ���Ƶ�Ĭ�������棬��������������
* deployrequirement ����Ԫ���ݣ�ÿ��taskgroup�ж��deployrequirement,�û����沿������
* resourcerequirement ��Դ����
```
                        service
                        /     \
                       /       \
         (Ĭ����)taskgroup0  taskgroup1(��ʱ�飬����С����)
         |       |    /   |    |     |    \
       task0    task1/  task2  task3 task4 \
                    /                       \
                  /                          \
                 /                            \
        deployrequirement              deployrequirement      
        |              |               |              |
resourcerequirement   ...     resourcerequirment     ...    

       
```
