#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreBluetooth/CBService.h>

@interface Utils : NSObject

+(void)showMessage:(NSString*)message;
+(const char*)UUIDToString:(CFUUIDRef)UUID;
+(UInt16)swap:(UInt16)s;
+(const char*)CBUUIDToString:(CBUUID*)UUID;
+(UInt16)CBUUIDToInt:(CBUUID*)UUID;

@end
