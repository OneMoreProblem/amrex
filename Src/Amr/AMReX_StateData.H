
#ifndef AMREX_StateData_H_
#define AMREX_StateData_H_

#include <memory>

#include <AMReX_Box.H>
#include <AMReX_BoxArray.H>
#include <AMReX_MultiFab.H>
#include <AMReX_MFCopyDescriptor.H>
#include <AMReX_BCRec.H>
#include <AMReX_Array.H>
#include <AMReX_Vector.H>
#include <AMReX_VisMF.H>
#include <AMReX_DistributionMapping.H>
#include <AMReX_PhysBCFunct.H>
#include <AMReX_Geometry.H>
#include <AMReX_RealBox.H>
#include <AMReX_StateDescriptor.H>

namespace amrex {

class StateDataPhysBCFunct;

//
// Current and previous level-time data.
//
// StateData holds state data on a level for the current and previous time step.
//

class StateData
{
    friend class StateDataPhysBCFunct;

public:
    //
    // The default constructor.
    //
    StateData ();
    //
    // Constructor that properly initializes data members.
    //
    StateData (const Box&             p_domain,
               const BoxArray&        grds,
	       const DistributionMapping& dm,
               const StateDescriptor* d,
               Real                   cur_time,
               Real                   dt,
               const FabFactory<FArrayBox>& factory);
    //
    // The destructor.
    //
    ~StateData ();

    StateData (StateData&& rhs) noexcept;
    StateData (StateData const& rhs) = delete;
    void operator= (StateData && rhs) = delete;
    void operator= (StateData const& rhs);

    //
    // Initializes data members if you used default constructor.
    //
    void define (const Box&             p_domain,
                 const BoxArray&        grds,
		 const DistributionMapping& dm,
                 const StateDescriptor& d, 
                 Real                   cur_time,
                 Real                   dt,
                 const FabFactory<FArrayBox>& factory);
    //
    // Copies old data from another StateData object and sets the same time level.
    // If old data is uninitialized, allocates it with same properties as the input data.
    //
    void copyOld (const StateData& state);
    //
    // Copies new data from another StateData object and sets the same time level.
    // If new data is uninitialized, allocates it with the same properties as the input data.
    //
    void copyNew (const StateData& state);
    //
    // Allocates space for old timestep data.
    //
    void allocOldData ();
    //
    // Deletes the space used by the old timestep data.
    //
    void removeOldData () { old_data.reset(); }
    //
    // Reverts back to initial state.
    //
    void reset ();
    //
    // Old data becomes new data and new time is incremented by dt.
    //
    void swapTimeLevels (Real dt);
    //
    // Swaps old data with a new MultiFab.
    // Deletes the previous old data.
    //
    void replaceOldData ( MultiFab&& mf );
    //
    // Swaps old data with another StateData.
    // Does not delete the previous old data.
    //
    void replaceOldData ( StateData& s );
    //
    // Swaps new data with a new MultiFab.
    // Deletes the previous new data.
    //
    void replaceNewData ( MultiFab&& mf );
    //
    // Swaps new data with another StateData.
    // Does not delete the previous new data.
    //
    void replaceNewData ( StateData& s );

    //
    // Sets time of old and new data.
    //
    void setTimeLevel (Real t_new,
                       Real dt_old,
                       Real dt_new);
    //
    // Sets time of old data.
    //
    void setOldTimeLevel (Real t_old);
    //
    // Sets time of new data.
    //
    void setNewTimeLevel (Real t_new);

    void syncNewTimeLevel (Real t_new);

    void RegisterData (MultiFabCopyDescriptor& multiFabCopyDesc,
                       Vector<MultiFabId>&      mfid);

    void InterpAddBox (MultiFabCopyDescriptor& multiFabCopyDesc,
		       Vector<MultiFabId>&      mfid,
		       BoxList*                returnedUnfillableBoxes,
		       Vector<FillBoxId>&       returnedFillBoxIds,
		       const Box&              subbox,
		       Real                    time,
		       int                     src_comp,
		       int                     dest_comp,
		       int                     num_comp,
		       bool                    extrap = false);

    void InterpFillFab (MultiFabCopyDescriptor&  fabCopyDesc,
			const Vector<MultiFabId>& mfid,
			const Vector<FillBoxId>&  fillBoxIds,
			FArrayBox&               dest,
			Real                     time,
			int                      src_comp,
			int                      dest_comp,
			int                      num_comp,
			bool                     extrap = false);

    //
    // Set physical bndry values
    //
    void FillBoundary (FArrayBox&     dest,
                       Real           time,
                       const Real*    dx,
                       const RealBox& prob_domain,
                       int            dest_comp,
                       int            src_comp,
                       int            num_comp = 1);

    void FillBoundary (Box const&      bx,
                       FArrayBox&      dest,
                       Real            time,
                       Geometry const& geom,
                       int             dest_comp,
                       int             src_comp,
                       int             num_comp);
    //
    // Write the state data to a checkpoint file.
    //
    void checkPoint (const std::string& name,
                     const std::string& fullpathname,
                     std::ostream&      os,
                     VisMF::How         how,
                     bool               dump_old = true);
    //
    // Restart with domain box, grids, and dmap provided
    //
    void restart (std::istream&          is,
		  const Box&             p_domain,
		  const BoxArray&        grds,
		  const DistributionMapping& dm,
                  const FabFactory<FArrayBox>& factroy,
                  const StateDescriptor& d,
                  const std::string&     restart_file);
    //
    // or from another similar state
    //
    void restart (const StateDescriptor& d,
		  const StateData& rhs);
    //
    // Returns the StateDescriptor.
    //
    const StateDescriptor* descriptor () const { return desc; }
    //
    // Returns the valid domain.
    //
    const Box& getDomain () const { return domain; }
    //
    // Returns the BoxArray.
    //
    const BoxArray& boxArray () const { return grids; }
    //
    const DistributionMapping& DistributionMap () const { return dmap; }
    void setDistributionMap ( DistributionMapping& new_dmap ) { dmap = new_dmap; }
    //
    const FabFactory<FArrayBox>& Factory () const { return *m_factory; }
    //
    // Returns the current time.
    //
    Real curTime () const {
	return (desc->timeType() == StateDescriptor::Point) ?
	    new_time.stop : 0.5*(new_time.start + new_time.stop);
    }
    //
    // Returns the previous time.
    //
    Real prevTime () const {
	return (desc->timeType() == StateDescriptor::Point) ?
	    old_time.stop : 0.5*(old_time.start + old_time.stop);
    }
    //
    // Returns the new data.
    //
    MultiFab& newData () { BL_ASSERT(new_data != nullptr); return *new_data; }
    //
    // Returns the new data.
    //
    const MultiFab& newData () const { BL_ASSERT(new_data != nullptr); return *new_data; }
    //
    // Returns the old data.
    //
    MultiFab& oldData () { BL_ASSERT(old_data != nullptr); return *old_data; }
    //
    // Returns the old data.
    //
    const MultiFab& oldData () const { BL_ASSERT(old_data != nullptr); return *old_data; }
    //
    // Returns the FAB of new data at grid index `i'.
    //
    FArrayBox& newGrid (int i) { BL_ASSERT(new_data != nullptr); return (*new_data)[i]; }
    //
    // Returns the FAB of old data at grid index `i'.
    //
    FArrayBox& oldGrid (int i) { BL_ASSERT(old_data != nullptr); return (*old_data)[i]; }
    //
    // Returns boundary conditions of specified component on the specified grid.
    //
    BCRec getBC (int comp, int i) const;
    //
    // Prints out the time interval.
    //
    void printTimeInterval (std::ostream& os) const;
    //
    // True if there is any old data available.
    //
    bool hasOldData () const { return old_data != nullptr; }
    //
    // True if there is any new data available.
    //
    bool hasNewData () const { return new_data != nullptr; }

    void getData (Vector<MultiFab*>& data,
		  Vector<Real>& datatime,
		  Real time) const;

    //
    // These facilitate prereading FabArray headers to avoid
    // synchronization when reading multiple FabArrays
    //
    static const Vector<std::string> &FabArrayHeaderNames() { return fabArrayHeaderNames; }
    static void ClearFabArrayHeaderNames() { fabArrayHeaderNames.clear(); }
    static void SetFAHeaderMapPtr(std::map<std::string, Vector<char> > *fahmp) { faHeaderMap = fahmp; }


private:

    std::unique_ptr<FabFactory<FArrayBox> > m_factory;

    struct TimeInterval
    {
        Real start;
        Real stop;
    };
    //
    // Pointer to data descriptor.
    //
    const StateDescriptor* desc;
    //
    // Problem domain.
    //
    Box domain;
    //
    // Grids defined at this level.
    //
    BoxArray grids;
    //
    DistributionMapping dmap;
    //
    // Time variable assoc with new data.
    //
    TimeInterval new_time;
    //
    // Time variable assoc with old data.
    //
    TimeInterval old_time;
    //
    // Pointer to new-time data.
    //
    std::unique_ptr<MultiFab> new_data;
    //
    // Pointer to previous time data.
    //
    std::unique_ptr<MultiFab> old_data;
    //
    // This is used as a temporary collection of FabArray header
    // names written during a checkpoint
    //
    static Vector<std::string> fabArrayHeaderNames;
    //
    // This is used to store preread FabArray headers
    //
    static std::map<std::string, Vector<char> > *faHeaderMap;  // ---- [faheader name, the header]

    void restartDoit (std::istream& is, const std::string& restart_file);
};

class StateDataPhysBCFunct
    : public PhysBCFunctBase
{
public:
    StateDataPhysBCFunct (StateData& sd, int sc, const Geometry& geom_);
    virtual void FillBoundary (MultiFab& mf, int dcomp, int ncomp, Real time, int bccomp) override;
    using PhysBCFunctBase::FillBoundary;
private:
    StateData* statedata;
    int src_comp;
    const Geometry& geom;
    
};

}

#endif /*_StateData_H_*/
