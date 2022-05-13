#ifndef _WANSON_API_H_
#define _WANSON_API_H_

#if defined(__XC__)
extern "C" {
#endif
    void wanson_task_create(unsigned priority, StreamBufferHandle_t audio_stream);
    int Wanson_ASR_Init();


    /*****************************************
    * Input:
    *      - domain  : 0 - wakeup
                       1 - asr
    *
    * Return value   : 0 - OK
    *                 -1 - Error
    ******************************************/
    int Wanson_ASR_Reset(int domain);



    /*****************************************
    * Input:
    *      - buf      : Audio data (16k, 16bit, mono)
    *      - buf_len  : Now must be 480 (30ms)
    *
    * Output:
    *      - text     : The text of ASR
    *      - score    : The confidence of ASR (Now not used)
    *
    * Return value    :  0 - No result
    *                    1 - Has result
    *                   -1 - Error
    ******************************************/
    int Wanson_ASR_Recog(short *buf, int buf_len, const char **text, int* id);

    void Wanson_ASR_Release();  //stop ASR Engine

#if defined(__XC__)
}
#endif

#endif // _WANSON_API_H_
