# 设计

buffer_t 和 parser 紧密耦合，与其他部件解耦，可以进行模拟测试。request_t包含这两者，让两者耦合
