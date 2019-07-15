def dynamic_crt():
    f1 = open('build\config\win\BUILD.gn','r+')
    infos = f1.readlines()
    f1.seek(0,0)
    for line in infos:
        line_new = line.replace('configs = [ ":static_crt" ]','configs = [ ":dynamic_crt" ]')
        f1.write(line_new)
    f1.close()
    return True

if __name__ == '__main__':
    if not dynamic_crt():
        print 'dynamic_crt failed'
        exit(1)
    
    exit(0)