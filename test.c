/*  
 *  This example writes data to the HDF5 file by rows.
 *  Number of processes is assumed to be 1 or multiples of 2 (up to 8)
 */
 
#include "hdf5.h"
#include "stdlib.h"
#include "H5FDsubfiling.h" /* Private header for the subfiling VFD */
#include "H5FDioc.h"


#define H5FILE_NAME     "SDS_row.h5"
#define DATASETNAME 	"IntArray" 
#define NX     65536                      /* dataset dimensions */
#define NY     8 
#define RANK   2

int
main (int argc, char **argv)
{
    /*
     * HDF5 APIs definitions
     */ 	
    hid_t       file_id, dset_id;         /* file and dataset identifiers */
    hid_t       filespace, memspace;      /* file and memory dataspace identifiers */
    hsize_t     dimsf[2];                 /* dataset dimensions */
    int         *data;                    /* pointer to data buffer to write */
    hsize_t	count[2];	          /* hyperslab selection parameters */
    hsize_t	offset[2];
    hid_t	plist_id;                 /* property list identifier */
    int         i;
    herr_t	status;

    /*
     * MPI variables
     */
    int mpi_size, mpi_rank;
    MPI_Comm comm  = MPI_COMM_WORLD;
    MPI_Info info  = MPI_INFO_NULL;

    /*
     * Initialize MPI
     */
    int required = MPI_THREAD_MULTIPLE;
    int provided = 0;

    MPI_Init_thread(&argc, &argv, required, &provided);

    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);  
 
    /* 
     * Set up file access property list with parallel I/O access
     */
     plist_id = H5Pcreate(H5P_FILE_ACCESS);
     hid_t ioc_fapl, fapl;
     H5FD_ioc_config_t ioc_config;
     H5FD_subfiling_config_t subfiling_conf;
   
     memset(&ioc_config, 0, sizeof(ioc_config));
     memset(&subfiling_conf, 0, sizeof(subfiling_conf));
            
#if 0 
     H5Pget_fapl_subfiling(plist_id, &subfiling_conf);
        
     ioc_fapl = H5Pcreate(H5P_FILE_ACCESS);
     H5Pget_fapl_ioc(ioc_fapl, &ioc_config);
     H5Pset_fapl_ioc(ioc_fapl, &ioc_config);
#endif
     
     H5Pset_fapl_subfiling(plist_id, NULL);
        
//        H5Pclose(ioc_fapl);

    /*
     * Create a new file collectively and release property list identifier.
     */
    file_id = H5Fcreate(H5FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    H5Pclose(plist_id);

   
    /*
     * Create the dataspace for the dataset.
     */
    dimsf[0] = NX;
    dimsf[1] = NY;
    filespace = H5Screate_simple(RANK, dimsf, NULL); 

    /*
     * Create the dataset with default properties and close filespace.
     */
    dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace,
			H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Sclose(filespace);

    /* 
     * Each process defines dataset in memory and writes it to the hyperslab
     * in the file.
     */
    count[0] = dimsf[0]/mpi_size;
    count[1] = dimsf[1];
    offset[0] = mpi_rank * count[0];
    offset[1] = 0;
    memspace = H5Screate_simple(RANK, count, NULL);

    /*
     * Select hyperslab in the file.
     */
    filespace = H5Dget_space(dset_id);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

    /*
     * Initialize data buffer 
     */
    data = (int *) malloc(sizeof(int)*count[0]*count[1]);
    for (i=0; i < count[0]*count[1]; i++) {
        data[i] = mpi_rank + 10;
    }

    /*
     * Create property list for collective dataset write.
     */
    
    status = H5Dwrite(dset_id, H5T_NATIVE_INT, memspace, filespace,
    		      H5P_DEFAULT, data);
    free(data);

    /*
     * Close/release resources.
     */
    H5Dclose(dset_id);
    H5Sclose(filespace);
    H5Sclose(memspace);

    H5Fclose(file_id);

    /* open the HDF5 file */

    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_subfiling(plist_id, NULL);

    file_id = H5Fopen (H5FILE_NAME, H5F_ACC_RDONLY, plist_id);
    dset_id = H5Dopen (file_id, DATASETNAME, H5P_DEFAULT);

    H5Dclose(dset_id);
    H5Fclose(file_id);
    H5Pclose(plist_id);

    MPI_Finalize();

    return 0;
}     

