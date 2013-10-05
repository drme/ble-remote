#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreBluetooth/CBService.h>

#define TELLY_SERVICE_ID    @"EEEE"
#define TELLY_TYPE_ID       @"EEE0"
#define TELLY_CODE_ID       0xeee1
#define TELLY_ADDRESS_ID    @"EEE2"

@interface TellySensor : NSObject<CBPeripheralDelegate>

@property (assign) int batteryLevel;
@property (assign) int rssiLevel;
@property (assign) NSString* deviceName;

-(id)initWithData:(CBPeripheral*)peripheral;
-(void)updateStatus;
-(void)sendPower;
-(void)sendHome;
-(void)sendSource;
-(void)sendBack;
-(void)sendVolumeUp;
-(void)sendVolumeDown;
-(void)sendOk;
-(void)sendLeft;
-(void)sendRight;
-(void)sendUp;
-(void)sendDown;
-(CBPeripheral*)getDevice;

 @end
