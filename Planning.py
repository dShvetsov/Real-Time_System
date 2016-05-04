# -*- coding: utf-8 -*-
"""
Created on Wed May  4 15:42:11 2016

@author: dshvetsov
"""
#to reading
import xml.etree.ElementTree as ET
#to writting
from  lxml import etree
import sys

def Limit(job) : 
    return job['finish_deadline'] - job['start_deadline']


def T_PL(jobs) : 
    """ return smallets time from start_deadlines """
    start_deadlines= [j['start_deadline'] for j in jobs]
    return min(start_deadlines)
     
def final (executed_jobs, unsched):
    """ dump result in file"""
    result_root = etree.Element('trace')
    for t, j in executed_jobs.iteritems():
        etree.SubElement(result_root, 'job', attrib={'name':j['name'], 'time':str(t)} )
    
    for j in unsched:
        etree.SubElement(result_root, 'unsched', attrib={'name':j['name']} )

    handle = etree.tostring(result_root, pretty_print=True)
    if (len(sys.argv) < 3):
        f = open('output.xml', 'w')
    else : 
        f = open(sys.argv[2])
    f.write(handle)
    f.close();
     
def main(name='input_static.xml') : 
    xml = ET.parse(name)
    jobs = xml.getroot()
    
    #verify xml file
    for j in jobs : 
        if j.tag != 'job' : 
            raise NameError('non job in xml')
        if j.keys() != ['duration', 'finish_deadline', 'name', 'start_deadline']:
            raise NameError('invalid attributes')

    # transform jobs to more comfortable format
    jobs = map(lambda j : j.attrib, jobs)    
    for j in jobs : 
        j['duration'] = int(j['duration'])
        j['finish_deadline'] = int(j['finish_deadline'])
        j['start_deadline' ] = int(j['start_deadline'])
        
    # delete all unexecuted jobs
    unsched = []
    unsched.extend(filter(lambda j : Limit(j) - j['duration'] <=0, jobs))
    jobs = filter(lambda j : Limit(j) - j['duration']  > 0, jobs)
    if (len(jobs) == 0) : 
        final({}, unsched)        
        return
    
    t_pl = T_PL(jobs)
    
    executed_jobs = {}
        
    while (len(jobs) > 0) : 
        #compute suitable jobs
        suit_jobs = filter(lambda j : j['start_deadline'] <= t_pl < j['finish_deadline'], jobs)
        if (len(suit_jobs) == 0):
            #if everyone not suit, recompute t_pl
            t_pl = T_PL(jobs)
            continue        
        suit_jobs = sorted(suit_jobs, key=lambda j : Limit(j) - j['duration'])
        #take best by second variant in task        
        job = suit_jobs[0]
        jobs.remove(job)
        executed_jobs[t_pl] = job
        t_pl += job['duration']
        #delete all jobs, which we can't execute
        unsched.extend(filter(lambda j : j['finish_deadline'] < j['duration'] + t_pl, jobs))
        jobs = filter(lambda j : j['finish_deadline'] >= j['duration'] + t_pl, jobs)
        
    final(executed_jobs, unsched)

    
    
if __name__ == '__main__':
    if (len(sys.argv) > 1) :  
        main(sys.argv[1])
    else : 
        main()