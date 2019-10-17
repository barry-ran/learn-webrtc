def dynamic_crt():
    with open(r'build\config\win\BUILD.gn', 'r+') as f1:
        infos = f1.readlines()
        if -1 == infos[404].find('configs = [ ":static_crt" ]'):
            print 'not find configs = [ ":static_crt" ]'
            return False
              
        line = infos[404].replace('configs = [ ":static_crt" ]', 'configs = [ ":dynamic_crt" ]')
        infos[404] = line

        f1.seek(0, 0)        
        f1.writelines(infos)
        return True
        
    return False    

def remove_capture_checker():    
    with open(r'modules\video_capture\windows\sink_filter_ds.cc', 'r+') as f1:
        infos = f1.readlines()
        if -1 == infos[722].find('RTC_DCHECK_RUN_ON(&capture_checker_);'):
            print 'not find RTC_DCHECK_RUN_ON(&capture_checker_);'
            return False
              
        line = infos[722].replace('RTC_DCHECK_RUN_ON(&capture_checker_);', '//RTC_DCHECK_RUN_ON(&capture_checker_);')
        infos[722] = line

        f1.seek(0, 0)        
        f1.writelines(infos)
        return True
        
    return False

if __name__ == '__main__':
    if not dynamic_crt():
        print 'dynamic_crt failed'
        exit(1)
    
    # 由于这个bug https://groups.google.com/forum/#!searchin/discuss-webrtc/$26capture_checker_$20iscurrent|sort:date/discuss-webrtc/HBzGZXL-FvM/SG2TSnaoBAAJ
    if not remove_capture_checker():
        print 'remove_capture_checker failed'
        exit(1)
    
    exit(0)