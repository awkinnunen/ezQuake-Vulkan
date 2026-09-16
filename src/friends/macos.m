/* OpenAI Codex, GPL-2.0-or-later. Launch Services owns macOS URL associations. */
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
int FriendsMac_Registered(void){
    CFStringRef current=LSCopyDefaultHandlerForURLScheme(CFSTR("ezquake-vulkan"));
    NSString *bundle=[[NSBundle mainBundle] bundleIdentifier];
    int same=current&&bundle&&CFStringCompare(current,(CFStringRef)bundle,0)==kCFCompareEqualTo;
    if(current)CFRelease(current);return same;
}
int FriendsMac_Register(int enable){
    if(!enable)return 0; /* macOS associations are managed by Launch Services. */
    NSString *bundle=[[NSBundle mainBundle] bundleIdentifier];if(!bundle)return 0;
    LSRegisterURL((CFURLRef)[[NSBundle mainBundle] bundleURL],true);
    return LSSetDefaultHandlerForURLScheme(CFSTR("ezquake-vulkan"),(CFStringRef)bundle)==noErr;
}
