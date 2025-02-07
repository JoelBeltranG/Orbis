from obspy import read, Stream
import seisbench
import seisbench.models as sbm
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
import pandas as pd


seisbench.use_backup_repository()
eq_model = sbm.EQTransformer.from_pretrained("stead")

directory = "/Users/roberto/SDS2/RAE96/"

df_acumulado = pd.DataFrame()
#122 - 289
for i in range(122,289):
    print(i)
    pattern_E = directory + "RAE96_EHE_2024_"+str(i)+".mseed"
    pattern_N = directory + "RAE96_EHN_2024_"+str(i)+".mseed"
    pattern_Z = directory + "RAE96_EHZ_2024_"+str(i)+".mseed"

    # Read files matching each pattern into their respective streams
    try:
        stream_E = read(pattern_E)
    except FileNotFoundError:
        print(f"File {pattern_E} not found")
        stream_E = None

    try:
        stream_N = read(pattern_N)
    except FileNotFoundError:
        print(f"File {pattern_N} not found")
        stream_N = None

    try:
        stream_Z = read(pattern_Z)
    except FileNotFoundError:
        print(f"File {pattern_Z} not found")
        stream_Z = None

    

    if stream_E is not None:
        if stream_N is not None:
            if stream_Z is not None:
                stream = stream_E + stream_N + stream_Z
    
    
    every = int(len(stream)/3)
    for i in range(0,every):
        selected_stream = Stream()
    # Add traces by specifying their index or other identifying feature
    # For example, if you know the indices:
        selected_stream += stream[i]  # First trace of component E
        selected_stream += stream[i+every]  # First trace of component N
        selected_stream += stream[i+2*every]  # First trace of component Z

        # Obtener el tiempo de inicio y fin común más corto entre las trazas
        tiempo_inicio_comun = max(traza.stats.starttime for traza in selected_stream)
        tiempo_fin_comun = min(traza.stats.endtime for traza in selected_stream)
        if tiempo_inicio_comun <= tiempo_fin_comun:
            # Recortar el stream al tiempo común
            stream_trimmed = selected_stream.slice(starttime=tiempo_inicio_comun, endtime=tiempo_fin_comun)

            stream_trimmed.detrend(type="linear")
            #stream_trimmed.normalize()
            outputs = eq_model.classify(stream_trimmed)
            print(outputs)
            ############Convirtiendo a dataframes
            df = pd.DataFrame(outputs.picks)
            if not df.empty:
                df = df[0].apply(lambda x: x.__dict__)
                df = pd.DataFrame(df.tolist())
                df.rename(columns={"start_time":'start_time_picks'},inplace=True)
                df.rename(columns={"end_time":'end_time_picks'},inplace=True)
                df_P = df[df['phase']=='P']
                df_S = df[df['phase']=='S']

                df2 = pd.DataFrame(outputs.detections)
                df2 = df2[0].apply(lambda x: x.__dict__)
                df2 = pd.DataFrame(df2.tolist())
                df2.rename(columns={"start_time":'start_time_detections'},inplace=True)
                df2.rename(columns={"end_time":'end_time_detections'},inplace=True)
                df2.rename(columns={"peak_value":'peak_value_detections'},inplace=True)
                ############## Merge de los dataframes
                final_merged_df = df2.merge(
                    df_P,  # Phase P
                    on="trace_id",
                    how="inner"
                ).merge(
                    df_S,  # Phase S
                    on="trace_id",
                    how="inner",
                    suffixes=("_P", "_S")
                ).query("start_time_detections <= end_time_picks_P <= end_time_detections and "
                        "start_time_detections <= end_time_picks_S <= end_time_detections")
                final_merged_df.reset_index(drop=True, inplace=True)

                final_merged_df['archivo'] = "RAE96_EHE_2024_"+str(i)+".mseed"
                #final_merged_df
                ##############################
                df_acumulado = pd.concat([df_acumulado, final_merged_df], ignore_index=True)


        else:
            # Si no hay solapamiento, puedes manejar el caso aquí (por ejemplo, imprimir un mensaje o continuar)
            print(f"Rango de tiempo inválido: {tiempo_inicio_comun}, {tiempo_fin_comun} ")

df_acumulado.to_csv("Picados_loscabos_eqtransformes.csv",index=False)      
