## Linux Process ����

### �㼶��ϵ

```
session --
         | 
          \group1 --
                   |
                    \process1 --
                   |            |
                    \process2    \ thread 1
                                |
                                 \ thread 2
```
ÿ���������Լ���id sid,gid,pid,tid 
### ����һ������
������һ������ʱ�����������gid���ӽ���Ĭ��ʹ�ø�����gid

### һ�����⣬�������Ľ���һ��process tree
������һ��taskʱ��task���ܻᴴ�����ӽṹ��process tree, ���һ��һ��processȥkill���鷳������ʹ��process groupȥkillȴ�ǳ�����
* fork һ��leader process
* ���ӽ�����������gid
* killpg
��������
```
#include <unistd.h>
#include <signal.h>
int main ()
{
    pid_t pid = fork();
    if(pid==0){
        FILE *fd = popen("python -m SimpleHTTPServer 12345","r");
        pclose(fd);
        return 0;
    }else{
        setpgid(pid,pid);
        killpg(pid,9);
    }   
    return 0;

}
```

### referenc
[processes](http://www.win.tue.nl/~aeb/linux/lk/lk-10.html)
                  
