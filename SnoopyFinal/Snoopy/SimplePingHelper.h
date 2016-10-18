#import <Foundation/Foundation.h>
#import "SimplePing.h"

@interface SimplePingHelper : NSObject <SimplePingDelegate>

+ (void)ping:(NSString*)address target:(id)target sel:(SEL)sel;

@end
