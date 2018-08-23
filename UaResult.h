//
// Created by wll on 2018/8/9.
//

#ifndef UDGER_UARESULT_H
#define UDGER_UARESULT_H
#include <string>
using std::string;

#define PropertyBuilderByName(type, name, access_permission)\
    access_permission:\
        type m_##name;\
    public:\
    inline void set##name(type v) {\
        m_##name = v;\
    }\
    inline type get##name() {\
        return m_##name;\
    }\

class UaResult
{
// UA
PropertyBuilderByName(int, classId, private);
PropertyBuilderByName(int, clientId, private);
PropertyBuilderByName(string, uaClass, private);
PropertyBuilderByName(string, uaClassCode, private);
PropertyBuilderByName(string, ua, private);
PropertyBuilderByName(string, uaEngine, private);
PropertyBuilderByName(string, uaVersion, private);
PropertyBuilderByName(string, uaVersionMajor, private);
PropertyBuilderByName(string, uaUptodateCurrentVersion, private);
PropertyBuilderByName(string, crawlerLastSeen, private);
PropertyBuilderByName(string, crawlerRespectRobotstxt, private);
PropertyBuilderByName(string, crawlerCategory, private);
PropertyBuilderByName(string, crawlerCategoryCode, private);
PropertyBuilderByName(string, uaFamily, private);
PropertyBuilderByName(string, uaFamilyCode, private);
PropertyBuilderByName(string, uaFamilyHomepage, private);
PropertyBuilderByName(string, uaFamilyIcon, private);
PropertyBuilderByName(string, uaFamilyIconBig, private);
PropertyBuilderByName(string, uaFamilyInfoUrl, private);
PropertyBuilderByName(string, uaFamilyVendor, private);
PropertyBuilderByName(string, uaFamilyVendorCode, private);
PropertyBuilderByName(string, uaFamilyVendorHomepage, private);
// OS
PropertyBuilderByName(string, osFamily, private);
PropertyBuilderByName(string, osFamilyCode, private);
PropertyBuilderByName(string, os, private);
PropertyBuilderByName(string, osCode, private);
PropertyBuilderByName(string, osHomePage, private);
PropertyBuilderByName(string, osIcon, private);
PropertyBuilderByName(string, osIconBig, private);
PropertyBuilderByName(string, osFamilyVendor, private);
PropertyBuilderByName(string, osFamilyVendorCode, private);
PropertyBuilderByName(string, osFamilyVendorHomepage, private);
PropertyBuilderByName(string, osInfoUrl, private);
// DEVICE
PropertyBuilderByName(string, deviceClass, private);
PropertyBuilderByName(string, deviceClassCode, private);
PropertyBuilderByName(string, deviceClassIcon, private);
PropertyBuilderByName(string, deviceClassIconBig, private);
PropertyBuilderByName(string, deviceClassInfoUrl, private);

PropertyBuilderByName(string, deviceMarketname, private);
PropertyBuilderByName(string, deviceBrand, private);
PropertyBuilderByName(string, deviceBrandCode, private);
PropertyBuilderByName(string, deviceBrandHomepage, private);
PropertyBuilderByName(string, deviceBrandIcon, private);
PropertyBuilderByName(string, deviceBrandIconBig, private);
PropertyBuilderByName(string, deviceBrandInfoUrl, private);
private:
    // UA
    string uaString;

public:
    UaResult(const string &uaString){
        this->uaString = uaString;
        this->setclientId(-1);
        this->setclassId(-1);
    }
    UaResult(){}
    string toString() {
        string result = "UdgerUaResult ["
                        "uaString=" + this->uaString +
                        ", clientId=" + std::to_string(this->getclientId()) +
                        ", classId=" + std::to_string(this->getclassId())+
                        ", uaClass=" + this->getuaClass()+
                        ", uaClassCode=" + this->getuaClassCode()+
                        ", ua=" + this->getua()+
                        ", uaEngine=" + this->getuaEngine()+
                        ", uaVersion=" + this->getuaVersion()+
                        ", uaVersionMajor=" + this->getuaVersionMajor()+
                        ", crawlerLastSeen=" + this->getcrawlerLastSeen()+
                        ", crawlerRespectRobotstxt=" + this->getcrawlerRespectRobotstxt()+
                        ", crawlerCategory=" + this->getcrawlerCategory()+
                        ", crawlerCategoryCode=" + this->getcrawlerCategoryCode()+
                        ", uaUptodateCurrentVersion=" + this->getuaUptodateCurrentVersion()+
                        ", uaFamily=" + this->getuaFamily()+
                        ", uaFamilyCode=" + this->getuaFamilyCode()+
                        ", uaFamilyHomepage=" + this->getuaFamilyHomepage()+
                        ", uaFamilyIcon=" + this->getuaFamilyIcon()+
                        ", uaFamilyIconBig=" +this->getuaFamilyIconBig() +
                        ", uaFamilyVendor=" +this->getuaFamilyVendor() +
                         ", uaFamilyVendorCode=" +this->getuaFamilyVendorCode() +
                         ", uaFamilyVendorHomepage=" + this->getuaFamilyVendorHomepage()+
                         ", uaFamilyInfoUrl=" + this->getuaFamilyInfoUrl()+
                         ", osFamily=" + this->getosFamily()+
                         ", osFamilyCode=" + this->getosFamilyCode()+
                         ", os=" + this->getos()+
                         ", osCode=" + this->getosCode()+
                         ", osHomePage=" +this->getosHomePage() +
                         ", osIcon=" + this->getosIcon()+
                         ", osIconBig=" + this->getosIconBig()+
                         ", osFamilyVendor=" + this->getosFamilyVendor()+
                         ", osFamilyVendorCode=" + this->getosFamilyVendorCode()+
                         ", osFamilyVendorHomepage=" + this->getosFamilyVendorHomepage()+
                         ", osInfoUrl=" + this->getosInfoUrl()+
                         ", deviceClass=" + this->getdeviceClass()+
                         ", deviceClassCode=" + this->getdeviceClassCode()+
                         ", deviceClassIcon=" + this->getdeviceClassIcon()+
                         ", deviceClassIconBig=" + this->getdeviceClassIconBig()+
                         ", deviceClassInfoUrl=" + this->getdeviceClassInfoUrl()+
                         ", deviceMarketname=" + this->getdeviceMarketname()+
                         ", deviceBrand=" + this->getdeviceBrand()+
                         ", deviceBrandCode=" +this->getdeviceBrandCode() +
                          ", deviceBrandHomepage=" + this->getdeviceBrandHomepage()+
                          ", deviceBrandIcon=" +this->getdeviceBrandIcon() +
                           ", deviceBrandIconBig=" + this->getdeviceBrandIconBig()+
                           ", deviceBrandInfoUrl=" +this->getdeviceBrandInfoUrl() +
                           "]";
        return result;
    }
};
#endif //UDGER_UARESULT_H
