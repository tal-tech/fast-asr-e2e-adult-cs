/*
 * tal_paraformer_api.h
 *
 *  Created on: Feb 16, 2023
 *      Author: wangyaoping
 */

#ifndef TAL_PARAFORMER_API_H_
#define TAL_PARAFORMER_API_H_

#include <vector>
#include <string>

#define EXPORT_API __attribute__((visibility("default")))
#ifdef __cplusplus
extern "C" {
#endif

/*
 * brief 资源初始化
 * @param [in] resource_dir, 资源路径
 * @param [out] resource_handle, 返回句柄, 异常时候返回"NULL"
 * @param [out] 资源初始化状态, 0表示初始化成功, 其它表示异常
 */
EXPORT_API int TalParaformerResourceImport(const char* resource_dir, void** resource_handle);

/*
 * brief  资源释放
 * @param [in] resource, 资源句柄
 */
EXPORT_API void TalParaformerResourceRelease(void* resource_handle);

/*
 * brief  获取ASR资源版本
 * @param [in] resource, ASR资源句柄
 * @param [out] ASR资源版本
 */
EXPORT_API std::string TalParaformerGetResourceVersion(void* resource_handle);

/*
 * brief  获取ASR SDK版本
 * @param [out] SDK版本
 */
EXPORT_API std::string TalParaformerGetSDKVersion();

/*
 * brief 创建一个ASR实例
 * @param [in] resource, 资源配置句柄
 * @param [out] asr_instance_handle, 实例句柄
 * @param [out] 状态值, 0表示创建成功, 其它表示失败
 */
EXPORT_API int TalParaformerInstanceCreate(void* resource_handle, void** asr_instance_handle);

/*
 * brief  ASR实例删除
 * @param [in] asr_instance_handle, ASR实例句柄
 */
EXPORT_API void TalParaformerInstanceDelete(void* asr_instance_handle);

/*
 * brief  执行ASR推理
 * @param [in] asr_instance_handle, ASR实例句柄
 * @param [in] wav_data, wav音频数据
 * @param [in] wav_size, wav音频数据长度
 * @param [out] result, ASR识别结果
 * @param [out] 状态值, 0表示本次处理正常, 其它表示异常
 */
EXPORT_API int TalParaformerInstanceRecognize(void* asr_instance_handle, const float* wav_data, const int wav_size, std::string& result);

#ifdef __cplusplus
};
#endif
#endif /* TAL_PARAFORMER_API_H_ */
