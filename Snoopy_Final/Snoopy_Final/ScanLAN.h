#import <Foundation/Foundation.h>

@protocol ScanLANDelegate <NSObject>

@optional
- (void)scanLANDidFindNewAdrress:(NSString *)address havingHostName:(NSString *)hostName;
- (void)scanLANDidFinishScanning;
@end

@interface ScanLAN : NSObject

@property(nonatomic,weak) id<ScanLANDelegate> delegate;

- (id)initWithDelegate:(id<ScanLANDelegate>)delegate;
- (void)startScan;
- (void)stopScan;
- (void)getUpnpDiscovery;
- (NSString *)getIPAddress;
- (NSString *)GetCurrentWifiHotSpotName;
@end
