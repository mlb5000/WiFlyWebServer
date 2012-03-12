#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief Send a packet onto the network

    @param data [in] The TCP data to put on the network
    @param size [in] The number of bytes to send
    
    @retval 0 Success
    @retval 1 Failure
**/
int writePacket(const char *data, size_t size);

/** @brief Read a packet off the network
    
    @param outBuf [out] Buffer to populate with the network data
    @param size [in/out] In - The maximum size of outBuf, Out - the number of bytes written
    @param timeout_ms [in] The number of milliseconds to wait for data
    
    @retval 0 Success, no data remaining
    @retval 1 Success, data remaining
    @retval 2 Timeout
    @retval 3 Error reading data
**/
int readPacket(const char *outBuf, size_t *size, unsigned timeout_ms);

#ifdef __cplusplus 
}
#endif

#endif