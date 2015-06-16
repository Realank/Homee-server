class RealankQueue:
    class Empty(Exception):
        pass
    def __init__(self):
        self.queue = []
    def pop(self,index=-1):
        if len(self.queue)==0 :
            raise self.Empty()
        else:
            return self.queue.pop(index)
    def put(self,item):
        self.queue.append(item)
    def __iter__(self):
        return self.queue.__iter__()
        


q = RealankQueue()
q.put(1)
q.put('ggftfgsd')
p = RealankQueue()
p.put(2)
p.put('ggftfrrrrgsd')

    
for a in q:
    print a
for a in p:
    print a
