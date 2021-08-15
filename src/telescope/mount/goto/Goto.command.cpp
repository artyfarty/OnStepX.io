//--------------------------------------------------------------------------------------------------
// telescope mount control, commands

#include "Goto.h"

#if defined(MOUNT_PRESENT) && SLEW_GOTO == ON

#include "../../../tasks/OnTask.h"
extern Tasks tasks;

#include "../site/Site.h"
#include "../Mount.h"
#include "../guide/Guide.h"
#include "../home/Home.h"
#include "../limits/Limits.h"

extern unsigned long periodSubMicros;

bool Goto::command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError) {
  PrecisionMode precisionMode = PM_HIGH;

  if (command[0] == 'A') {
    // :AW#       Align Write to EEPROM
    //            Returns: 1 on success
    if (command[1] == 'W' && parameter[0] == 0) {
      #if ALIGN_MAX_NUM_STARS > 1  
        transform.align.modelWrite();
      #endif
    } else

    // :A?#       Align status
    //            Returns: mno#
    //            where m is the maximum number of alignment stars
    //                  n is the current alignment star (0 otherwise)
    //                  o is the last required alignment star when an alignment is in progress (0 otherwise)
    if (command[1] == '?' && parameter[0] == 0) {
      reply[0] = '0' + ALIGN_MAX_NUM_STARS;
      reply[1] = '0' + alignState.currentStar;
      reply[2] = '0' + alignState.lastStar;
      reply[3] = 0;
      *numericReply = false;
    } else

    // :A[n]#     Start Telescope Manual Alignment Sequence
    //            This is to initiate a n-star alignment for 1..MAX_NUM_ALIGN_STARS:
    //            1) Before calling this function, the telescope should be in the polar-home position
    //            2) Call this function with the # of align stars you'd like to use
    //            3) Set the target location (RA/Dec) to a bright star, etc. (not too near the NCP/SCP)
    //            4) Issue a goto command
    //            5) Center the star/object using the guide commands (as needed)
    //            6) Call :A+# command to accept the correction
    //            ( for two+ star alignment )
    //            7) Back to #3 above until done, except where possible choose at least one star on both meridian sides
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] >= '1' && command[1] <= ALIGN_MAX_NUM_STARS + '0' && parameter[0] == 0) {
      // set current time and date before calling this routine
      home.reset();
      mount.tracking(true);

      // start align...
      alignState.lastStar = command[1] - '0';
      alignState.currentStar = 1;
      VLF("MSG: Mount, align started");
    } else

    // :A+#       Align accept target location
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == '+' && parameter[0] == 0) {
      if (alignState.lastStar > 0 && alignState.currentStar <= alignState.lastStar) {
        CommandError e = alignAddStar();
        if (e != CE_NONE) {
          alignState.lastStar = 0;
          alignState.currentStar = 0;
          *commandError = e;
          DLF("ERR: Mount, failed to add align point");
        } else { VLF("MSG: Mount, align point added"); }
      } else *commandError = CE_ALIGN_NOT_ACTIVE;
    } else *commandError = CE_CMD_UNKNOWN;
  } else

  //  C - Sync Control
  // :CS#       Synchonize the telescope with the current right ascension and declination coordinates
  //            Returns: Nothing (Sync's fail silently)
  // :CM#       Synchonize the telescope with the current database object (as above)
  //            Returns: "N/A#" on success, "En#" on failure where n is the error code per the :MS# command
  if (command[0] == 'C' && (command[1] == 'S' || command[1] == 'M') && parameter[0] == 0) {
    CommandError e;
    if (alignState.lastStar > 0 && alignState.currentStar <= alignState.lastStar) {
      e = alignAddStar();
      if (e != CE_NONE) { alignState.lastStar = 0; alignState.currentStar = 0; *commandError = e; }
    } else {
      PierSideSelect pps = preferredPierSide;
      if (!mount.isHome() && PIER_SIDE_SYNC_CHANGE_SIDES == OFF) pps = PSS_SAME_ONLY;
      e = requestSync(&gotoTarget, pps);
    }
    if (command[1] == 'M') {
      if (e >= CE_SLEW_ERR_BELOW_HORIZON && e <= CE_SLEW_ERR_UNSPECIFIED) strcpy(reply,"E0");
      reply[1] = (char)(e - CE_SLEW_ERR_BELOW_HORIZON) + '1';
      if (e == CE_NONE) strcpy(reply,"N/A");
    }
    *numericReply = false;
  } else

  if (command[0] == 'G') {
    // :Gr#       Get target Right Ascension
    //            Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
    // :GrH#      Returns: HH:MM:SS.SSSS# (high precision)
    if (command[1] == 'r' && (parameter[0] == 0 || parameter[1] == 0)) {
      if (parameter[0] == 'H') precisionMode = PM_HIGHEST; else if (parameter[0] != 0) { *commandError = CE_PARAM_FORM; return true; }
      convert.doubleToHms(reply, radToHrs(gotoTarget.r), false, precisionMode);
      *numericReply = false;
    } else

    // :Gd#       Get target Declination
    //            Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
    // :GdH#      High precision
    //            Returns: HH:MM:SS.SSS# (high precision)
    if (command[1] == 'd' && (parameter[0] == 0 || parameter[1] == 0)) {
      if (parameter[0] == 'H') precisionMode = PM_HIGHEST; else if (parameter[0] != 0) { *commandError = CE_PARAM_FORM; return true; }
      convert.doubleToDms(reply, radToDeg(gotoTarget.d), false, true, precisionMode);
      *numericReply = false;
    } else

    // :Ga#       Get target Altitude
    //            Returns: sDD*MM# or sDD*MM'SS# (based on precision setting)
    // :GaH#      High precision
    //            Returns: sDD*MM'SS.SSS# (high precision)
    if (command[1] == 'a' && (parameter[0] == 0 || parameter[1] == 0)) {
      if (parameter[0] == 'H') precisionMode = PM_HIGHEST; else if (parameter[0] != 0) { *commandError = CE_PARAM_FORM; return true; }
      convert.doubleToHms(reply, radToDeg(gotoTarget.a), false, precisionMode);
      *numericReply = false;
    } else

    // :Gz#       Get target Azimuth
    //            Returns: DDD*MM# or DDD*MM'SS# (based on precision setting)
    // :GzH#      High precision
    //            Returns: DDD*MM'SS.SSS# (high precision)
    if (command[1] == 'z' ) {
      if (parameter[0] == 'H') precisionMode = PM_HIGHEST; else if (parameter[0] != 0) { *commandError = CE_PARAM_FORM; return true; }
      convert.doubleToDms(reply, radToDeg(gotoTarget.z), false, true, precisionMode);
      *numericReply = false;
    } else

    if (command[1] == 'X' && parameter[2] == 0)  {
      #if ALIGN_MAX_NUM_STARS > 1
        // :GX0[n]#   Get align setting [n]
        //            Returns: Value
        if (parameter[0] == '0')  {
          static int star = 0;
          *numericReply = false;
          switch (parameter[1]) {
            case '0': sprintf(reply,"%ld",(long)(transform.align.model.ax1Cor*3600.0F)); break; // ax1Cor
            case '1': sprintf(reply,"%ld",(long)(transform.align.model.ax2Cor*3600.0F)); break; // ax2Cor
            case '2': sprintf(reply,"%ld",(long)(transform.align.model.altCor*3600.0F)); break; // altCor
            case '3': sprintf(reply,"%ld",(long)(transform.align.model.azmCor*3600.0F)); break; // azmCor
            case '4': sprintf(reply,"%ld",(long)(transform.align.model.doCor*3600.0F));  break; // doCor
            case '5': sprintf(reply,"%ld",(long)(transform.align.model.pdCor*3600.0F));  break; // pdCor
            case '6': if (transform.mountType == FORK || transform.mountType == ALTAZM)         // ffCor
              sprintf(reply,"%ld",(long)(transform.align.model.dfCor*3600.0F)); else sprintf(reply,"%ld",(long)(0));
            break;
            case '7': if (transform.mountType != FORK && transform.mountType != ALTAZM)         // dfCor
              sprintf(reply,"%ld",(long)(transform.align.model.dfCor*3600.0F)); else sprintf(reply,"%ld",(long)(0));
            break;
            case '8': sprintf(reply,"%ld",(long)(transform.align.model.tfCor*3600.0F)); break;  // tfCor
            // number of stars, reset to first star
            case '9': { int n = 0; if (alignState.currentStar > alignState.lastStar) n = alignState.lastStar; sprintf(reply,"%ld",(long)(n)); star = 0; } break;
            case 'A': { convert.doubleToHms(reply,radToHrs(transform.align.actual[star].ax1),true,PM_HIGH); } break;
            case 'B': { convert.doubleToDms(reply,radToDeg(transform.align.actual[star].ax2),false,true,PM_HIGH); } break;
            case 'C': { convert.doubleToHms(reply,radToHrs(transform.align.mount[star].ax1),true,PM_HIGH); } break;
            case 'D': { convert.doubleToDms(reply,radToDeg(transform.align.mount[star].ax2),false,true,PM_HIGH); } break;
            // pier side (and increment n)
            case 'E': sprintf(reply,"%ld",(long)(transform.align.mount[star].side)); star++; break;
            default: *numericReply = true; *commandError = CE_CMD_UNKNOWN;
          }
        } else
      #endif

      // :GX9[n]#   Get setting [n]
      //            Returns: Value
      if (parameter[0] == '9')  {
        Coordinate current;
        *numericReply = false;
        switch (parameter[1]) {
          case '2': sprintF(reply, "%0.3f", settings.usPerStepCurrent); break;     // current
          case '3': sprintF(reply, "%0.3f", usPerStepBase); break;                 // default base
          // pierSide 0 = None, 1 = East, 2 = West (with suffix 'N' if meridian flips are disabled)
          case '4':
              current = mount.getMountPosition();
              sprintf(reply, "%d%s", (int)current.pierSide, (!transform.meridianFlips)?" N":"");
          break;
          case '5': sprintf(reply, "%d", (int)settings.meridianFlipAuto); break;   // autoMeridianFlip
          case '6': reply[0] = "EWB"[preferredPierSide - 10]; reply[1] = 0; break; // preferred pier side
          case '7': sprintF(reply, "%0.1f", (1000000.0F/settings.usPerStepCurrent)/degToRadF(axis1.getStepsPerMeasure())); break;
          // rotator availablity 2 = rotate/derotate, 1 = rotate, 0 = off
          case '8':
            if (AXIS3_DRIVER_MODEL != OFF) {
              if (transform.mountType == ALTAZM) strcpy(reply, "D"); else strcpy(reply, "R");
            } else strcpy(reply, "N");
          break;
          case '9': sprintF(reply, "%0.3f",usPerStepLowerLimit()); break;          // fastest step rate in us
          default: return false;
        }
      } else return false;
    } else return false;
  } else

  if (command[0] == 'M') {
    // :MA#       Goto the target Alt and Az
    //            Returns: 0..9, see :MS#
    if (command[1] == 'A' && parameter[0] == 0) {
      CommandError e = setTarget(&gotoTarget, preferredPierSide);
      if (e >= CE_SLEW_ERR_BELOW_HORIZON && e <= CE_SLEW_ERR_UNSPECIFIED) reply[0] = (char)(e - CE_SLEW_ERR_BELOW_HORIZON) + '1';
      if (e == CE_NONE) reply[0] = '0';
      reply[1] = 0;
      *numericReply = false; 
      *supressFrame = true;
      *commandError = e;
    } else

    // :MD#       Goto Destination pier side for the Target Object
    //            Returns:
    //              0=destination is East of the pier
    //              1=destination is West of the pier
    //              2=an error occured
    if (command[1] == 'D' && parameter[0] == 0) {
      CommandError e = setTarget(&gotoTarget, preferredPierSide);
      strcpy(reply, "2");
      if (e == CE_NONE && target.pierSide == PIER_SIDE_EAST) reply[0] = '0';
      if (e == CE_NONE && target.pierSide == PIER_SIDE_WEST) reply[0] = '1';
      *numericReply = false;
      *supressFrame = true;
      *commandError = e;
    } else

    //  :MN#   Goto current RA/Dec but East of the Pier (within meridian limit overlap for GEM mounts)
    //  :MNe#  as above
    //  :MNw#  Goto current RA/Dec but West of the Pier (within meridian limit overlap for GEM mounts)
    //         Returns: 0..9, see :MS#
    if (command[1] == 'N') {
      if (transform.mountType != ALTAZM) {
        Coordinate newTarget = mount.getPosition();
        CommandError e;

        if (parameter[0] == 0 ||
           (parameter[0] == 'e' && parameter[1] == 0)) e = request(&newTarget, PSS_EAST_ONLY); else
        if (parameter[0] == 'w' && parameter[1] == 0) e = request(&newTarget, PSS_WEST_ONLY); else e = CE_CMD_UNKNOWN;

        if (e != CE_CMD_UNKNOWN) {
          if (e >= CE_SLEW_ERR_BELOW_HORIZON && e <= CE_SLEW_ERR_UNSPECIFIED) reply[0] = (char)(e - CE_SLEW_ERR_BELOW_HORIZON) + '1';
          if (e == CE_NONE) reply[0] = '0';
          reply[1] = 0;
          *numericReply = false;
          *supressFrame = true;
        }
        *commandError = e;
      } else *commandError = CE_CMD_UNKNOWN;
    } else

    // :MP#       Goto the Current Position for Polar Align
    //            Returns: 0..9, see :MS#
    if (command[1] == 'P' && parameter[0] == 0) {
      if (transform.mountType != ALTAZM) {
        Coordinate newTarget = mount.getPosition();
        CommandError e = validate();
        if (e == CE_NONE) e = limits.validateCoords(&newTarget);
        if (e == CE_NONE) {
          #if ALIGN_MAX_NUM_STARS > 1
            transform.align.model.altCor = 0.0;
            transform.align.model.azmCor = 0.0;
          #endif
          e = request(&newTarget, PSS_SAME_ONLY);
        }
        if (e >= CE_SLEW_ERR_BELOW_HORIZON && e <= CE_SLEW_ERR_UNSPECIFIED) reply[0] = (char)(e - CE_SLEW_ERR_BELOW_HORIZON) + '1';
        if (e == CE_NONE) reply[0] = '0';
        reply[1] = 0;
        *numericReply = false;
        *supressFrame = true;
        *commandError = e;
      } else *commandError = CE_CMD_UNKNOWN;
    } else

    // :MS#       Goto the Target Object
    //            Returns:
    //              0=Goto is possible
    //              1=below the horizon limit
    //              2=above overhead limit
    //              3=controller in standby
    //              4=mount is parked
    //              5=Goto in progress
    //              6=outside limits (AXIS2_LIMIT_MAX, AXIS2_LIMIT_MIN, AXIS1_LIMIT_MIN/MAX, MERIDIAN_E/W)
    //              7=hardware fault
    //              8=already in motion
    //              9=unspecified error
    if (command[1] == 'S' && parameter[0] == 0) {
      CommandError e = request(&gotoTarget, preferredPierSide);
      strcpy(reply,"0");
      if (e >= CE_SLEW_ERR_BELOW_HORIZON && e <= CE_SLEW_ERR_UNSPECIFIED) reply[0] = (char)(e - CE_SLEW_ERR_BELOW_HORIZON) + '1';
      if (e == CE_NONE) reply[0] = '0';
      *numericReply = false;
      *supressFrame = true;
      *commandError = e;
    } else return false;
  } else

  if (command[0] == 'S') {
    //  :Sr[HH:MM.T]# or :Sr[HH:MM:SS]# or :Sr[HH:MM:SS.SSSS]#
    //            Set Target Right Ascension
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'r') {
      if (!convert.hmsToDouble(&gotoTarget.r, parameter)) *commandError = CE_PARAM_RANGE; else
      gotoTarget.r = hrsToRad(gotoTarget.r);
    } else

    //  :Sd[sDD*MM]# or :Sd[sDD*MM:SS]# or :Sd[sDD*MM:SS.SSS]#
    //            Set Target Declination
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'd') {
      if (!convert.dmsToDouble(&gotoTarget.d, parameter, true)) *commandError = CE_PARAM_RANGE; else
      gotoTarget.d = degToRad(gotoTarget.d);
    } else

    //  :Sa[sDD*MM]# or :Sa[sDD*MM'SS]# or :Sa[sDD*MM'SS.SSS]#
    //            Set Target Altitude
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'a') {
      if (!convert.dmsToDouble(&gotoTarget.a, parameter, true)) *commandError = CE_PARAM_RANGE; else
      gotoTarget.a = degToRad(gotoTarget.a);
    } else

    //  :Sz[DDD*MM]# or :Sz[DDD*MM'SS]# or :Sz[DDD*MM'SS.SSS]#
    //            Set Target Azmuith
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'z') {
      if (!convert.dmsToDouble(&gotoTarget.z, parameter, false)) *commandError = CE_PARAM_RANGE; else
      gotoTarget.z = degToRad(gotoTarget.z);
    } else

    if (command[1] == 'X') {
      if (parameter[2] != ',') { *commandError = CE_PARAM_FORM; return true; }

      #if ALIGN_MAX_NUM_STARS > 1
        // :SX0[m],[n]#   Set align model coefficient [m] to value [n]
        //                Return: 0 on failure
        //                        1 on success
        if (parameter[0] == '0') {
          static int star;
          double d;
          switch (parameter[1]) {
            case '0': transform.align.model.ax1Cor = atol(&parameter[3])/3600.0F; break; // ax1Cor
            case '1': transform.align.model.ax2Cor = atol(&parameter[3])/3600.0F; break; // ax2Cor 
            case '2': transform.align.model.altCor = atol(&parameter[3])/3600.0F; break; // altCor
            case '3': transform.align.model.azmCor = atol(&parameter[3])/3600.0F; break; // azmCor
            case '4': transform.align.model.doCor = atol(&parameter[3])/3600.0F; break;  // doCor
            case '5': transform.align.model.pdCor = atol(&parameter[3])/3600.0F; break;  // pdCor
            case '6': if (transform.mountType == FORK || transform.mountType == ALTAZM)
              transform.align.model.dfCor = atol(&parameter[3])/3600.0F; break;          // fdCor or ffCor
            break;
            case '7': if (transform.mountType != FORK && transform.mountType != ALTAZM)
              transform.align.model.dfCor = atol(&parameter[3])/3600.0F; break;          // fdCor or ffCor
            break;
            case '8': transform.align.model.tfCor = atol(&parameter[3])/3600.0F; break;  // tfCor
            // use :SX09,0# to start upload of stars for align, when done use :SX09,1# to calculate the pointing model
            case '9': {
                int n = atol(&parameter[3]);
                if (n == 1 && star >= 2) {
                  alignState.lastStar = star;
                  alignState.currentStar = star + 1;
                  transform.align.createModel(star);
                } else star = 0;
              }
            break;
            // Star #n HA
            case 'A': if (!convert.hmsToDouble(&d, &parameter[3], PM_HIGH)) *commandError = CE_PARAM_FORM; else transform.align.actual[star].ax1 = hrsToRad(d); break;
            // Star #n Dec
            case 'B': if (!convert.dmsToDouble(&d, &parameter[3], true, PM_HIGH)) *commandError = CE_PARAM_FORM; else transform.align.actual[star].ax2 = degToRad(d); break;
            // Mount #n HA
            case 'C': if (!convert.hmsToDouble(&d, &parameter[3], PM_HIGH)) *commandError = CE_PARAM_FORM; else transform.align.mount[star].ax1 = hrsToRad(d); break;
            // Star #n Dec
            case 'D': if (!convert.dmsToDouble(&d, &parameter[3], true, PM_HIGH)) *commandError = CE_PARAM_FORM; else transform.align.mount[star].ax2 = degToRad(d); break;
            // Mount PierSide (and increment n)
            case 'E': transform.align.actual[star].side = transform.align.mount[star].side = atol(&parameter[3]); star++; break;
            default: *commandError = CE_CMD_UNKNOWN;
          }
        } else
      #endif

      // :SX9[m],[n]#   Set mount misc.
      //                Return: see below
      if (parameter[0] == '9') {
        switch (parameter[1]) {
          // set new slew rate (returns 1 success or 0 failure)
          case '2':
            if (state == GS_NONE && guide.state == GU_NONE) {
              char *conv_end;
              settings.usPerStepCurrent = strtod(&parameter[3], &conv_end);
              if (settings.usPerStepCurrent < usPerStepBase/2.0) settings.usPerStepCurrent = usPerStepBase/2.0;
              if (settings.usPerStepCurrent > usPerStepBase*2.0) settings.usPerStepCurrent = usPerStepBase*2.0;
              if (settings.usPerStepCurrent < usPerStepLowerLimit()) settings.usPerStepCurrent = usPerStepLowerLimit();
              nv.updateBytes(NV_MOUNT_GOTO_BASE, &settings, sizeof(GotoSettings));
              updateAccelerationRates();
            } else *commandError = CE_SLEW_IN_MOTION;
          break;
          // slew rate preset (returns nothing)
          case '3':
            *numericReply = false;
            if (state == GS_NONE && guide.state == GU_NONE) {
              switch (parameter[3]) {
                case '5': settings.usPerStepCurrent = usPerStepBase*2.0; break; // 50%
                case '4': settings.usPerStepCurrent = usPerStepBase*1.5; break; // 75%
                case '3': settings.usPerStepCurrent = usPerStepBase;     break; // 100%
                case '2': settings.usPerStepCurrent = usPerStepBase/1.5; break; // 150%
                case '1': settings.usPerStepCurrent = usPerStepBase/2.0; break; // 200%
                default:  settings.usPerStepCurrent = usPerStepBase;
              }
              if (settings.usPerStepCurrent < usPerStepLowerLimit()) settings.usPerStepCurrent = usPerStepLowerLimit();
              nv.updateBytes(NV_MOUNT_GOTO_BASE, &settings, sizeof(GotoSettings));
              updateAccelerationRates();
            } else *commandError = CE_SLEW_IN_MOTION;
          break;
          // autoMeridianFlip
          case '5':
            if (parameter[3] == '0' || parameter[3] == '1') {
              settings.meridianFlipAuto = parameter[3] - '0';
              #if MFLIP_AUTOMATIC_MEMORY == ON 
                nv.updateBytes(NV_MOUNT_GOTO_BASE, &settings, sizeof(GotoSettings));
              #endif
            } else *commandError = CE_PARAM_RANGE;
          break;
          // preferred pier side
          case '6':
            switch (parameter[3]) {
              case 'E': preferredPierSide = PSS_EAST; break;
              case 'W': preferredPierSide = PSS_WEST; break;
              case 'B': preferredPierSide = PSS_BEST; break;
              default: *commandError = CE_PARAM_RANGE;
            }
          break;
          // pause at home on meridian flip
          case '8':
            if (parameter[3] == '0' || parameter[3] == '1') {
              settings.meridianFlipPause = parameter[3] - '0';
              #if MFLIP_PAUSE_HOME_MEMORY == ON
                nv.updateBytes(NV_MOUNT_GOTO_BASE, &settings, sizeof(GotoSettings));
              #endif
            } else *commandError = CE_PARAM_RANGE;
          break;
          // continue if paused at home
          case '9':
            if (parameter[3] == '1') {
              if (meridianFlipHome.paused) meridianFlipHome.resume = true;
            } else *commandError = CE_PARAM_RANGE;
          break;
          default: return false;
        }
      } else return false;
    } else return false;
  } else return false;

  return true;
}

#endif