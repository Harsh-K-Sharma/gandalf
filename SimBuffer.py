from collections import deque
import fnmatch
import os
from SphSim import SphSimulation, SphSnapshot

# ============================================================================
# CLASS SIMBUFFER
# ============================================================================
class SimBuffer:
    '''
    This class is intended both to act as a cache for the snapshots and to
    provide routines to create new simulation instances.
    At the moment, it stores a list of the created simulations and a queue of the
    stored snapshots. Each snapshot holds a reference to the parent simulation,
    while each simulation holds a dictionary, listing all the snapshot files that
    have been found on the disk, and whether they are cached.
    To keep these structures synchronized, it is better to use only the methods
    provided by this class to modify its members (reading new snapshots and
    simulations and deleting them). 
    '''

    Nsim = int(0)
    simlist = []
    snapshots = deque()
    maxmemory = 1024**3
    currentsim = -1


    # ========================================================================
    # SIMBUFFER __INIT__
    # ========================================================================
    @staticmethod
    def _add_simulation(sim):

        SimBuffer.simlist.append(sim)
        SimBuffer.Nsim += 1
        SimBuffer.currentsim = SimBuffer.Nsim - 1
    
    # TODO: eliminate duplicate code between this function and fillsnapshot
    # TODO: improve the algorithm for making space/allocating space
    @staticmethod        
    def _addsnapshot(sim, snapshot):
        '''
        Add a new snapshot to the list of snapshots.
        Checks for memory usage, and deallocates as many snapshots as needed
        for making space. Updates the reference
        to the parent simulation accordingly. Raises BufferFull if there isn't enough space
        for the snapshot being loaded. 
        '''
        while 1:
            snapshotsize = snapshot.CalculateMemoryUsage()
            if snapshotsize > SimBuffer.maxmemory:
                print "Warning: the requested snapshot can't fit inside the buffer memory. It will not be cached"
                raise BufferFull
            available_memory = SimBuffer.maxmemory - SimBuffer.total_memory_usage()
            if snapshotsize > available_memory:
                SimBuffer._makespace()
            else:
                break
        SimBuffer.snapshots.append(snapshot)
        snapshot.sim = sim
    
    @staticmethod
    def _fillsnapshot(snapshot):
        print "Inside fillsnapshot"
        snapshot.ReadSnapshot(snapshot.sim.simparams.stringparams["in_file_form"],snapshot.sim)
        snapshotsize = snapshot.CalculateMemoryUsage()
        if snapshotsize > SimBuffer.maxmemory:
            print "Warning: the requested snapshot can't fit inside the buffer memory. It will not be cached"
            raise BufferFull
        while 1:
            available_memory = SimBuffer.maxmemory - SimBuffer.total_memory_usage()
            if snapshotsize > available_memory:
                SimBuffer._makespace()
            else:
                break
    
    @staticmethod
    def _makespace():
        ''' Deletes the first snapshot on the queue (that is, the first created)
        and deallocates its memory.
        '''
        for snapshot in SimBuffer.snapshots:
            if snapshot.allocated:
                snapshot.DeallocateBufferMemory()
                return 
    
    @staticmethod
    def _destroy():
        '''Deallocates all the snapshots and clears the snapshots and simulation lists'''
        for snapshot in SimBuffer.snapshots:
            if snapshot.allocated:
                snapshot.DeallocateBufferMemory()
        SimBuffer.snapshots = deque()
        SimBuffer.simlist = []
        
    @staticmethod        
    def loadsim (run_id, fileformat = 'ascii', buffer_flag = 'cache'):
        '''
        This method loads a simulation into the buffer.
        
        Takes the run_id (can also be a relative path), the fileformat (read from
        the parameters file) and a string that flags how the buffer should behave
        with respect to memory management. Possible values are:
            
            cache
                (the default) When the simulation is loaded, only the first snapshot is read.
                When a new snapshot is requested, it is saved into memory so to be able to quickly
                retrieve it.
            store
                When the simulation is loaded, all the snapshots are loaded in memory, up to filling
                the memory buffer available. It requires a longer start-up time, but then accessing
                the snapshot in memory will be very fast.
            nocache
                No caching of the snapshots is done; the snapshots are always re-read from the disk.
        
        '''
        
        #construct the simulation object and initialize it
        paramfile = run_id + '.param'
        run_id_base = os.path.basename(run_id)
        sim = SphSimulation()
        SimBuffer._add_simulation(sim)
        parameters = sim.simparams
        parameters.ReadParamsFile(paramfile)
        sim.ProcessParameters()
        fileformat = parameters.stringparams["in_file_form"]
        
        #get the list of all files in the directory where the parameter file is
        paramfilepath = os.path.join(os.getcwd(),paramfile)
        dirname = os.path.dirname(paramfilepath)
        folderfiles = os.listdir(dirname)
        folderfiles.sort()
        
        #search for all the files containing the given run_id string
        filetest = run_id_base + '.' + fileformat + '.?????'
        sim.snapshots = []
        for filename in folderfiles:
            if fnmatch.fnmatch(filename, filetest):
                sim.snapshots.append(SphSnapshot(os.path.join(dirname,filename)))              
        if len(sim.snapshots) == 0:
            print "Warning: no snapshots files found for simulation " + run_id 
            
        for snapshot in sim.snapshots:
            print snapshot.filename      
        
        #depending on buffer_flag, caches all the snapshots or not
        for i, snapshot in enumerate(sim.snapshots):
            if buffer_flag == "store":
                snapshot.ReadSnapshot(fileformat, sim)
            elif buffer_flag == "cache":
                if i==0:
                    snapshot.ReadSnapshot(fileformat, sim)
            try:
                SimBuffer._addsnapshot(sim, snapshot)
            except BufferFull:
                snapshot.DeallocateBufferMemory()
                
        sim.current = sim.snapshots[0]
    
    @staticmethod  
    def total_memory_usage():
        '''
        This function return the total memory usage of the buffer (in bytes).
        '''
        total_memory = 0
        for snapshot in SimBuffer.snapshots:
            total_memory += snapshot.CalculateMemoryUsage()
        return total_memory
    
    @staticmethod  
    def get_current_sim():
        '''This function returns the current simulation'''
        return SimBuffer.simlist[SimBuffer.currentsim]
    
    @staticmethod  
    def get_current_snapshot_by_sim(sim):
        '''This function queries the simulation given and returns its current snapshot'''
        return sim.current
    
    @staticmethod      
    def get_current_snapshot():
        '''This function returns the current snapshot of the current simulation'''
        sim = SimBuffer.get_current_sim()
        return SimBuffer.get_current_snapshot_by_sim(sim)
    
    @staticmethod  
    def get_snapshot_number_sim(sim, no):
        '''This function returns the snapshot number from the given simulation'''
        snap = sim.snapshots[no]
        sim.current = snap
        if (not snap.allocated):
#            snap.ReadSnapshot(sim.simparams.stringparams["in_file_form"], sim)
            SimBuffer._fillsnapshot(snap)
        return snap
    
    @staticmethod
    def get_snapshot_number_current_sim(no):
        '''This function returns the snapshot number from the current simulation'''
        sim = SimBuffer.get_current_sim()
        return SimBuffer.get_snapshot_number_sim(sim, no)
    
    @staticmethod  
    def get_next_snapshot_sim (sim):
        '''This function returns the next snapshot from the given simulation'''
        index = sim.snapshots.index(sim.current)
        index += 1
        snap = SimBuffer.get_snapshot_number_sim(sim, index)
        return snap
    
    @staticmethod
    def get_next_snapshot_current():
        '''This function returns the next current snapshot'''
        sim = SimBuffer.get_current_sim()
        return SimBuffer.get_next_snapshot_sim (sim)
    
    @staticmethod
    def get_no_next_snapshot():
        '''This function returns the number of the next snapshot, without modifying it'''
        sim = SimBuffer.get_current_sim()
        return sim.snapshots.index(sim.current)+1
         
        

class BufferFull (Exception):
    pass