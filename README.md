## keepalived-windows

`startup.bat` 启动服务

`stop.bat` 停止服务

`reload.bat` 重载服务 修改配置后请重载服务。

### 配置说明

`ha.conf`

```conf
global_defs {
   vrrp_mcast_group4 224.100.100.100
}
vrrp_instance {
    state MASTER
    interface 192.168.64.128
    interface_name Ethernet0
    virtual_router_id 51
    priority 121
    advert_int 1
    virtual_ipaddress {
        192.168.64.200
        192.168.64.201
    }
}
```



- `global_defs` 全局配置
  + `vrrp_mcast_group4` 组播地址
- `vrrp_instance` 节点配置
  + `state` 当前节点初始化状态 可选值（MASTER | BACKUP）
  + `interface` 绑定网卡ip
  + `interface_name` 绑定网卡连接名 (ipconfig查看)
  + `virtual_router_id` 虚拟路由id，同一集群id值请设置一致 范围0到254
  + `priority` 权重，谁最大谁最终竞争胜利，state变为MASTER BACKUP的权重设置要小于MASTER 范围0到254
  + `advert_int` 心跳间隔 单位秒 范围0到254
  + `virtual_ipaddress` 虚拟ip，请保证在同一子网范围内，可配置多个

## 遇到问题？

请提交[issue](https://github.com/jarryxy/keepalived-windows/issues/new/choose)... 
