/* OpenAI Codex, GPL-2.0-or-later. No shell, no arbitrary IPC commands. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "windows.h"
#include "transport.h"

static HANDLE inbox=INVALID_HANDLE_VALUE;
static const char registration[]="Software\\Classes\\ezquake-vulkan";
static int FullBase(const char *base,char *out,DWORD capacity){
 DWORD n=GetFullPathNameA(base,capacity,out,NULL);
 return n&&n<capacity&&(GetFileAttributesA(out)&FILE_ATTRIBUTE_DIRECTORY)&&GetFileAttributesA(out)!=INVALID_FILE_ATTRIBUTES;
}
static int Command(const char *base,char *out,size_t size){
 char exe[1024],dir[1024];DWORD n=GetModuleFileNameA(NULL,exe,sizeof(exe));
 if(!n||n>=sizeof(exe)||!FullBase(base,dir,sizeof(dir)))return 0;
 /* Invitation MUST remain the last argument. Startup rejects appended args
    before any engine initialization, eliminating quote/command injection. */
 return snprintf(out,size,"\"%s\" -nohome -basedir \"%s\" +set vid_renderer 2 -friends-invite \"%%1\"",exe,dir)>0;
}
static int ReadCommand(char *out,DWORD size){
 DWORD type=0;HKEY key;LONG e;
 if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Classes\\ezquake-vulkan\\shell\\open\\command",0,KEY_READ,&key)!=ERROR_SUCCESS)return 0;
 e=RegQueryValueExA(key,NULL,NULL,&type,(BYTE*)out,&size);RegCloseKey(key);
 return e==ERROR_SUCCESS&&type==REG_SZ&&size>0&&out[size-1]==0;
}
int FriendsWindows_Registered(const char *base){char expected[2300],current[2300];return Command(base,expected,sizeof(expected))&&ReadCommand(current,sizeof(current))&&!_stricmp(expected,current);}
int FriendsWindows_Register(const char *base,int enable){
 char command[2300];HKEY key,open;LONG e;
 if(!enable){if(!FriendsWindows_Registered(base))return 0;e=RegDeleteTreeA(HKEY_CURRENT_USER,registration);if(e==ERROR_SUCCESS)SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,NULL,NULL);return e==ERROR_SUCCESS;}
 if(!Command(base,command,sizeof(command)))return 0;
 if(RegCreateKeyExA(HKEY_CURRENT_USER,registration,0,NULL,0,KEY_WRITE,NULL,&key,NULL)!=ERROR_SUCCESS)return 0;
 e=RegSetValueExA(key,NULL,0,REG_SZ,(const BYTE*)"URL:ezQuake Vulkan invitation",sizeof("URL:ezQuake Vulkan invitation"));
 if(e==ERROR_SUCCESS)e=RegSetValueExA(key,"URL Protocol",0,REG_SZ,(const BYTE*)"",1);
 if(e==ERROR_SUCCESS){
  e=RegCreateKeyExA(key,"shell\\open\\command",0,NULL,0,KEY_WRITE,NULL,&open,NULL);
  if(e==ERROR_SUCCESS){e=RegSetValueExA(open,NULL,0,REG_SZ,(const BYTE*)command,(DWORD)strlen(command)+1);RegCloseKey(open);}
 }
 RegCloseKey(key);if(e==ERROR_SUCCESS)SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,NULL,NULL);return e==ERROR_SUCCESS;
}
static int PipeName(const char *base,char *out,size_t size){
 char path[1024],user[256];DWORD n=sizeof(user);uint64_t hash=14695981039346656037ull;size_t i;
 if(!FullBase(base,path,sizeof(path))||!GetUserNameA(user,&n))return 0;
 for(i=0;path[i];++i)if(path[i]=='/')path[i]='\\';
 for(i=strlen(path);i>3&&path[i-1]=='\\';--i)path[i-1]=0;
 CharLowerBuffA(path,(DWORD)strlen(path));
 for(i=0;user[i];++i){hash^=(unsigned char)user[i];hash*=1099511628211ull;}
 hash^=0xff;hash*=1099511628211ull;
 for(i=0;path[i];++i){hash^=(unsigned char)path[i];hash*=1099511628211ull;}
 return snprintf(out,size,"\\\\.\\mailslot\\ezquake-vulkan-friends-%016llx",(unsigned long long)hash)>0;
}
int FriendsWindows_Forward(const char *base,const char *link){
 char name[160];HANDLE pipe;DWORD n=0,size;BOOL ok;
 if(!NF_ValidateInvite(link)||!PipeName(base,name,sizeof(name)))return 0;
 pipe=CreateFileA(name,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(pipe==INVALID_HANDLE_VALUE)return 0;
 size=(DWORD)strlen(link)+1;ok=WriteFile(pipe,link,size,&n,NULL);CloseHandle(pipe);return ok&&n==size;
}
void FriendsWindows_Init(const char *base){char name[160];if(PipeName(base,name,sizeof(name)))inbox=CreateMailslotA(name,512,0,NULL);}
void FriendsWindows_Poll(void){
 char data[512];DWORD count,next,n;
 if(inbox==INVALID_HANDLE_VALUE||!GetMailslotInfo(inbox,NULL,&next,&count,NULL)||!count||next>sizeof(data))return;
 if(ReadFile(inbox,data,sizeof(data),&n,NULL)&&n>1&&data[n-1]==0&&strlen(data)==n-1)Friends_ReceiveInvitation(data);
}
void FriendsWindows_Close(void){if(inbox!=INVALID_HANDLE_VALUE)CloseHandle(inbox);inbox=INVALID_HANDLE_VALUE;}
