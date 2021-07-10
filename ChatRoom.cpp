//
// Created by feng on 2021/5/13.
//

#include "ChatRoom.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>

#include "utils.h"
#include "config.h"


void message_body::decode_head(u_char *messagehead_r) {
    length = static_cast<u_int32_t >(messagehead_r[5] + (messagehead_r[4]<<8) + (messagehead_r[3]<<16) + (messagehead_r[2]<<24));
    type = static_cast<TYPE>(messagehead_r[0] & 0x0f);
    head = static_cast<u_int8_t >(messagehead_r[0]) >>4;
    is_group = static_cast<bool>(messagehead_r[1]&0x01);
    aes_encrypt_length = get_aes_encrypt_length(length);

}

std::pair<u_char *, size_t> message_body::encode(const AES_KEY *encrypt_key) {
    // 这个函数执行完，data就是秘文了

    aes_encrypt_length = get_aes_encrypt_length(length);  //密文长度

    size_t lengthdata = 46+aes_encrypt_length;

    u_char *returndata = (u_char *)calloc(lengthdata, 1);

    ssl_encrypt(data, returndata+46, length, encrypt_key);
    free(data);

    returndata[0] = type + (VERSION<<4);
    returndata[1] = is_group?0x01:0x00;
    returndata[2] = length >>24;
    returndata[3] = length >>16;
    returndata[4] = length >>8;
    returndata[5] = length & 0xff ;

    memcpy(returndata+6, sender_.c_str(), sender_.length());
    memcpy(returndata+26, receiver_.c_str(), receiver_.length());

    data = returndata+46;

    return {returndata, lengthdata};
}

ChatRoom::ChatRoom(std::string receiver_, std::string sender_, bool is_group_):
            receiver(receiver_), sender(sender_), is_group(is_group_)
            {
                std::string path = std::string(get_current_dir_name());
                path.append("/chatlog/").append(sender_).append("-").append(receiver_).append(".log");
                int fd = open(path.c_str(), O_CREAT|O_RDWR|O_APPEND, S_IRUSR | S_IWUSR);
                close(fd);
                fs.open(path, std::ios_base::in|std::ios_base::out|std::ios::app);
            }

void ChatRoom::init() {
    // TODO 获取本地记录
    load();
}

void ChatRoom::load() {// 获取本地记录
    if (fs.is_open()){
        std::string line;
        if(fs.fail()){
            std::cerr<<"error oprening file myname!"<<std::endl;
            exit(-1);
        }
        while(getline(fs,line))
            std::cout<<line<<std::endl;
        fs.clear();
        fs.seekp(ios::beg);   //应该可以删除这个的，
        std::cout<<"load history success"<<std::endl;

    }else{
        std::cout<<"log文本打开失败"<<std::endl;
    }
}

void ChatRoom::insert(std::string str) {
    // 将消息写入文本log
    if (fs.is_open()){
        if(fs.fail()){
            std::cerr<<"error opening file myname!"<<std::endl;
        }
        fs<<str;
        fs.flush();
    }else{
        std::cout<<"log文本打开失败"<<std::endl;
    }
}

void
ssl_encrypt(const unsigned char *in, unsigned char *out, size_t in_length, const AES_KEY *key) {
    unsigned char ivec[16];

    AES_cbc_encrypt(in, out, in_length, key, ivec, AES_ENCRYPT);
}

void
ssl_decrypt(const unsigned char *in, unsigned char *out, size_t in_length, const AES_KEY *key) {
    unsigned char ivec[16];

    AES_cbc_encrypt(in, out, in_length, key, ivec, AES_DECRYPT);
}
