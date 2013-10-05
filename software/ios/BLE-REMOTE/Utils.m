#import "Utils.h"

@implementation Utils

+(void)showMessage:(NSString*)message
{
    UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Error" message:message delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
    
    [alert show];
}

+(const char*)UUIDToString:(CFUUIDRef)UUID
{
    if (nil == UUID)
    {
        return "NULL";
    }
    
    return CFStringGetCStringPtr(CFUUIDCreateString(NULL, UUID), 0);
}

+(UInt16)swap:(UInt16)s
{
    UInt16 temp = s << 8;

    temp |= (s >> 8);
    
    return temp;
}

+(const char*)CBUUIDToString:(CBUUID*)UUID
{
    return [[UUID.data description] cStringUsingEncoding:NSStringEncodingConversionAllowLossy];
}

+(UInt16)CBUUIDToInt:(CBUUID*)UUID
{
    char b1[16];

    [UUID.data getBytes:b1];
    
    return ((b1[0] << 8) | b1[1]);
}

@end
