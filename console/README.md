### ���������
galaxy console��ǰ�˺ͺ�˷ֿ������ģ���μ����գ�ʹ�ù���proxy.go

#### ������˷���
```
cd backend && sh localrun.sh
```

#### ����ui

```
cd ui && grunt serve
```

#### ����proxy.go

��Ϊǰ�������������������Ҫһ��reverse proxy
```
go build proxy.go && ./proxy -conf=conf -host=http://0.0.0.0:8234 -port=8080
```

