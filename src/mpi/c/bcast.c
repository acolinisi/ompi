/*
 * $HEADERS$
 */
#include "ompi_config.h"
#include <stdio.h>

#include "mpi.h"
#include "mpi/c/bindings.h"
#include "mca/coll/coll.h"
#include "communicator/communicator.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILING_DEFINES
#pragma weak MPI_Bcast = PMPI_Bcast
#endif

#if OMPI_PROFILING_DEFINES
#include "mpi/c/profile/defines.h"
#endif

static const char FUNC_NAME[] = "MPI_Bcast";


int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype,
              int root, MPI_Comm comm)
{
    int err;

    if (MPI_PARAM_CHECK) {
      OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
      if (ompi_comm_invalid(comm)) {
	return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, 
                                     FUNC_NAME);
      }

      /* Errors for all ranks */

      if (MPI_DATATYPE_NULL == datatype) {
	return OMPI_ERRHANDLER_INVOKE(comm, MPI_ERR_TYPE, FUNC_NAME);
      }
    
      if (count < 0) {
	return OMPI_ERRHANDLER_INVOKE(comm, MPI_ERR_COUNT, FUNC_NAME);
      }

      /* Errors for intracommunicators */

      if (OMPI_COMM_IS_INTRA(comm)) {
        if ((root >= ompi_comm_size(comm)) || (root < 0)) {
          return OMPI_ERRHANDLER_INVOKE(comm, MPI_ERR_ROOT, FUNC_NAME);
        }
      } 

      /* Errors for intercommunicators */

      else {
        if (! ((root >= 0 && root < ompi_comm_remote_size(comm)) ||
               root == MPI_ROOT || root == MPI_PROC_NULL)) {
	  return OMPI_ERRHANDLER_INVOKE(comm, MPI_ERR_ROOT, FUNC_NAME);
        }
      } 
    }

    /* If there's only one node, we're done */

    if (OMPI_COMM_IS_INTRA(comm) && ompi_comm_size(comm) <= 1) {
      return MPI_SUCCESS;
    }

    /* Can we optimize? */

    if (count == 0 && comm->c_coll.coll_bcast_optimization) {
      return MPI_SUCCESS;
    }

    /* Invoke the coll component to perform the back-end operation */

    err = comm->c_coll.coll_bcast(buffer, count, datatype, root, comm);
    OMPI_ERRHANDLER_RETURN(err, comm, err, FUNC_NAME);
}
