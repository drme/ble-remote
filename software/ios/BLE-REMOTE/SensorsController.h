#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreBluetooth/CBService.h>
#import "TellySensor.h"

@protocol SensorsSearchDelegate

-(void)connected;
-(void)disconnected;

@end

@interface SensorsController : NSObject<CBCentralManagerDelegate>

@property (nonatomic,assign) id<SensorsSearchDelegate> delegateDiscover;
@property (strong, nonatomic) TellySensor* connectedRemote;
@property (strong) CBPeripheral* connectingPeripheral;

-(void)startDeviceSearch;
+(SensorsController*)getInstance;
-(void)disconnectActive;

@end
