# -*- coding: utf-8 -*-
"""
Created on Wed May  4 15:42:11 2016

@author: dshvetsov
"""

import xml.etree.ElementTree as ET
import sys

def Limit(job) : 
    return job['finish_deadline'] - job['start_deadline']

#def KeyFunc (job):
#    return Limit(job) - job['duration']

def T_PL(jobs) : 
    start_deadlines= [j['start_deadline'] for j in jobs]
    return min(start_deadlines)
     
     
def main(name='input_static.xml') : 
    xml = ET.parse(name)
    jobs = xml.getroot()
    
    #verify xml file
    for job in jobs : 
        if job.tag != 'job' : 
            raise NameError('non job in xml')
        if job.keys() != ['duration', 'finish_deadline', 'name', 'start_deadline']:
            raise NameError('invalid attributes')

    
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
        suit_jobs = filter(lambda j : j['start_deadline'] <= t_pl < j['finish_deadline'], jobs)
        suit_jobs = sorted(suit_jobs, key=lambda j : Limit(j) - job['duration'])
        if (len(suit_jobs) == 0):
            t_pl = T_PL(jobs)
            continue
        job = suit_jobs[0]
        jobs.remove(job)
        executed_jobs[t_pl] = job
        t_pl = t_pl + job['duration']
        #delete all jobs, which we can't execute
        unsched.extend(filter(lambda j : j['finish_deadline'] < j['duration'] + t_pl, jobs))
        jobs = filter(lambda j : j['finish_deadline'] >= j['duration'] + t_pl, jobs)
        
    final(executed_jobs, unsched)
    result_root = ET.Element('trace')
    for t, job in executed_jobs.iteritems():
        xmljob = ET.SubElement(result_root, 'job')
    
    
if __name__ == '__main__':
    main(sys.argv[1])