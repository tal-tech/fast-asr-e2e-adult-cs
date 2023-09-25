#include "alg/include/tal_paraformer_api.h"
#include "alg/include/wav.h"
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include "base/base64.h"

using namespace std;


shared_ptr<float> str2float(const std::string& data,bool& flag, int& data_size) {
    flag = false;
    // 除2是因为根据16的帧率要将1字节的char转换为2字节的int16  from：wav.h  算法demo使用方法
    data_size = data.size() / 2;
    int offset = 0;
    std::shared_ptr<float> data_(new float[data_size],
        [](float *p)
        { 
            if(p) {
                delete[] p;
                p = nullptr;
            }
        }
    );

    for(int i=0;i<data_size;++i) {
        int16_t sample;
        memcpy(&sample, data.data() + offset, sizeof(int16_t));
        offset += sizeof(int16_t);
        data_.get()[i] = static_cast<float>(sample);
    }
    flag = data_ ? true : false;
    return data_;
    
}

string getResult(string str)
{
    string result = "";
    int index1 = str.find("\"result\":");
    int index2 = str.find("\"sdk_version\":");
    for(int i = index1+9;i<index2-1;i++)
    {
        result.push_back(str[i]);
    }
    return result;
}

int main()
{
    const char *mod_dir = "../../res";
    void *asr_resource{nullptr};
    if (TalParaformerResourceImport(mod_dir, &asr_resource) || !asr_resource)
    {

        cout << "failed to load alg mod:" << mod_dir<<endl;
        exit(1);
    }
    void *dec{nullptr};
    if (TalParaformerInstanceCreate(asr_resource, &dec) || !dec)
    {
        cout << "asr create failed:" << endl;
    }
    std::string result_json;
    int ret = -1;
    bool is_chn = true;
    string version = TalParaformerGetResourceVersion(asr_resource);
    int data_size;
    bool flag;
    int fd = open("../../1_base.txt",O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error opening file: " << "1_base64.txt" << std::endl;
        return 1;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        std::cerr << "Error getting file size." << std::endl;
        close(fd);
        return 1;
    }
    char *strdata = static_cast<char*>(mmap(NULL,file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if(strdata == MAP_FAILED)
    {
        cout<<"mmap error"<<endl;
        close(fd);
        return 1;
    }
    string data(strdata);
    string database64;
    if(!base::Base64Decode(data, &database64)){
        cout<<"base64 error"<<endl;            
    }
    std::shared_ptr<float> res_data = str2float(database64, flag, data_size);
    cout << "flag: " << flag << endl;

    if(flag && res_data) {
        ret = TalParaformerInstanceRecognize(dec, res_data.get(), data_size, result_json); // 中文 中英
        if(ret < 0){
            cout<< "err_msg:" << "base64 decode error!"<<endl;
        }
    }
    cout<<ret<<endl;
    string resstr("R\"("+result_json+")\"");
    string result = getResult(resstr);
    cout<<result<<endl;
    return 0;

}