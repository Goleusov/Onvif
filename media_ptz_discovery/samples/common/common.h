//
// Created by allforgot on 2020/3/28.
//

#ifndef ONVIFCLIENT_GSOAP_COMMON_H
#define ONVIFCLIENT_GSOAP_COMMON_H

//#define USERNAME "peeklio"
//#define PASSWORD "Gdhuemk"
#define USERNAME "admin"
#define PASSWORD "admin"
//#define HOSTNAME "http://192.168.0.114/onvif/device_service"
//#define HOSTNAME "http://10.24.72.33:8899"
//#define HOSTNAME "http://10.24.72.33:8899//onvif/device_service"

//#define HOSTNAME "http://10.24.72.22:8080/onvif/devices"
#define HOSTNAME "http://10.21.200.6:554"

//#define HOSTNAME "http://10.24.72.22:8080/onvif/ptz"
//#define HOSTNAME "http://10.24.72.22:8080//onvif/device_service"
//#define HOSTNAME "http://10.24.72.22:8080/onvif/imaging"



//#define PROFILETOKEN "000"
#define PROFILETOKEN "MainProfileToken"

inline void CLEANUP_SOAP(struct soap* soap) {
    if (soap != nullptr) {
        soap_destroy(soap);
        soap_end(soap);
        soap_free(soap);
    }
}

inline void report_error(struct soap *soap) {
    std::cerr << "Oops, something went wrong:" << std::endl;
    soap_stream_fault(soap, std::cerr);
    //exit(EXIT_FAILURE);
}


#endif //ONVIFCLIENT_GSOAP_COMMON_H
