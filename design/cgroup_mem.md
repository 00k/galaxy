## Cgroup Memory ��ϵͳʹ���Լ������ļ�����

### ��ʹ��

#### �ں˰汾
3.10.0-123.4.2.el7.x86_64

#### ����һ��memory group
����֮ǰȷ���Ѿ�mount cgroup
`mkdir /sys/fs/cgroup/memory/0
echo $$ > /sys/fs/cgroup/memory/0/tasks
echo 4M > /sys/fs/cgroup/memory/0/memory.limit_in_bytes##G,M
cat /sys/fs/cgroup/memory/0/memory.limit_in_bytes
4194304`

### ��Ҫ�����ļ�����

#### task �ļ�
����ļ�������Ҫ����Ľ���,�߳�id,����Ϊ *��д* ,�������Ҫ��һ�����̼��뵽һ��cgroup��������޸�����ļ�,
��ȻϵͳҲ��ı�����ļ�������һ�������������� *�����߳�* ���� *fork����* �������task�ļ���

#### cgroup.procs �ļ�
����ļ�������Ҫ����Ľ���id,�ڵͰ汾�ں�����������Ϊ *ֻ��* ,����2.6.32_1-15-0-0,�����ڸ߰汾�ں�������
������ *��д*, �ڵ�ǰ���ڸ߰汾�ں˵� [mesos](http://mesos.apache.org/) ,[libcontainer](https://github.com/docker/libcontainer) �����޸�����ļ��������ڵͰ汾�ں�ʹ�����ǻ�������

#### memory.limit_in_bytes�ļ�
����ļ���ʾ��ǰcgroup����ܹ�ʹ�õ��ڴ��С,�ļ�����Ϊ *��д*,����ͨ���޸Ĵ��ļ���̬���������ڴ����ƣ���Ӧ�ôﵽ�ڴ����Ƶ�ʱ�򣬻����[OOM]()

#### memory.soft_limit_in_bytes�ļ�
�� memory.limit_in_bytes������ǣ�Ӧ�ÿ��Գ���memory.soft_limit_in_bytes��������Ӧ�ù���һ�����ڴ档���ǵ�ϵͳ��Դ���ŵ�ʱ��cgroup���Ӧ�õ��ڴ潵�͵�soft_limit_in_bytes�������

#### memory.stat �ļ�
����Ҫ����ڴ�ʹ�������ʱ�����ʹ������ļ���������㵱ǰ�ڴ�ʹ����� mem_used=rss+cache(+swap),memory.usage_in_bytesҲ���ڴ�ʹ����������������׼ȷ�������[memory controller](https://www.kernel.org/doc/Documentation/cgroups/memory.txt)

#### OOM Control
TODO
#### cgroup.event_control �ļ�
�¼�֪ͨ�ļ�, ��memory��ϵͳ�и����ܾ�������һ���ڴ淧ֵ����Ӧ���ڴ�ﵽ���ֵ�����Դ�����ļ���ȡ�¼�֪ͨ















