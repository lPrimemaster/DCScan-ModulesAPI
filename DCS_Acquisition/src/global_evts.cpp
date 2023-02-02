#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"

#include <queue>
#include <thread>
#include <atomic>

static std::thread* evt_loop_dcs_thread = nullptr;
static std::thread* evt_loop_mca_thread = nullptr;
static std::thread* evt_loop_cli_thread = nullptr;

static std::atomic<int> evt_loop_sig;

#define EVT_LOOP_RUN  1
#define EVT_LOOP_STOP 0
#undef max

void DCS::DAQ::StartEventLoop(DCS::u16 mca_num_channels)
{
    evt_loop_sig.store(EVT_LOOP_RUN);
    evt_loop_dcs_thread = new std::thread(DCSCountEvent);
    evt_loop_mca_thread = new std::thread(MCACountEvent, mca_num_channels);
    evt_loop_cli_thread = new std::thread(ClinometerEvent);
}

void DCS::DAQ::StopEventLoop()
{
    evt_loop_sig.store(EVT_LOOP_STOP);
    NotifyUnblockEventLoop();
    evt_loop_dcs_thread->join();
    evt_loop_mca_thread->join();
    evt_loop_cli_thread->join();

    delete evt_loop_dcs_thread;
    delete evt_loop_mca_thread;
    delete evt_loop_cli_thread;
}


void DCS::DAQ::DCSCountEvent()
{
    // TODO : Discriminate voltage levels here also. Ruling out unwanted X-ray counts.
    while(evt_loop_sig.load())
    {
        InternalVoltageData ivd = GetLastDCS_IVD();

        if(ivd.cr.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }

        DCSCountEventData evt_data;
        evt_data.counts = ivd.cr.num_detected;
        evt_data.measured_angle = ivd.measured_angle;
        evt_data.timestamp = ivd.timestamp;
        
        DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(DCSCountEventData)); // HACK : This can operate with a move ctor instead
    }
}

void DCS::DAQ::MCACountEvent(DCS::u16 mca_num_channels)
{
    while (evt_loop_sig.load())
    {
        InternalVoltageData ivd = GetLastMCA_IVD();

        if (ivd.cr.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }

        MCACountEventData evt_data;
        evt_data.count = ivd.cr.num_detected;
        evt_data.timestamp = ivd.timestamp;
        evt_data.total_max_bin_count = mca_num_channels;

        if (evt_data.count > 0)
        {
            for (int i = 0; i < evt_data.count; i++)
            {
                // TODO : Don't hard code DAQ range... (currently: [0.0, 10.0] V)
                evt_data.bins[i] = u16((ivd.cr.maxima[i] / 10.0) * evt_data.total_max_bin_count);
            }
        }

        DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(MCACountEventData)); // HACK : This can operate with a move ctor instead
    }
}

void DCS::DAQ::ClinometerEvent()
{
    while(evt_loop_sig.load())
    {
        InternalVoltageData ivd = GetLastClinometer_IVD();

        if(ivd.cr.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }
        

        // Guardar os valores de saída aqui
        // TODO: Falta colocar membros nesta estrutura (declarada no ficheiro DCS_ModuleAcquisition.h:88)
             

        ClinometerEventData evt_data;
        
        const float max_angle = 10;
        const float min_angle = -10;
        float range_angle = max_angle - min_angle; 

            for (int i = 0 ; i < 500 ; ++i)
            {
                evt_data.list_cliX[i] = 0.2*ivd.ptr[i]*range_angle + min_angle;
                evt_data.list_cliY[i] = 0.2*ivd.ptr[i+500]*range_angle + min_angle;
            }

        // TODO
        // Código para ler as samples do inclinómetro e fazer cálculos
        // O array ivd.ptr tem os últimos 500 pontos digitalizados da daq em valor de tensão de 0 a 5V para cada eixo medido.
        // i.e. ivd.ptr[0 a 499] = digit. Axis0, ivd.ptr[500 a 999] = digit. Axis1 (mesmo nome que no projeto DCScan-GUI)
        // Também têm acesso ao tempo do primeiro valor do array através de ivd.timestamp, o tempo dos valores seguintes do array
        // é calculado com base na taxa de aquisição, i.e. (se a taxa de aquisição for de 1000 amostras/s cada valor está espaçado
        // 1/1000 s, caso seja necessário ter essa granularidade)
        
        DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(ClinometerEventData)); // HACK : This can operate with a move ctor instead
    }
}
