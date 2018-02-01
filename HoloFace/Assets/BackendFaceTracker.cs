using System;

#if WINDOWS_UWP
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Graphics.Imaging;
using FaceProcessing;

namespace FaceProcessing
{
    public class BackendFaceTracker
    {
        private float confidenceThreshold;
        private int nLandmarks;

        private StreamSocketListener listener;
        private StreamSocket clientSocket;
        private DataWriter writer;
        private DataReader reader;

        private string port;
        private LocalFaceTracker localFaceTracker;
        private bool faceTracked = false;
        private float[] trackedLandmarks;
        private float[] canonicalLandmarks;
        private bool bResetModelFitter = true;
        private bool bConnected = false;
        private bool bInitialized = false;

        public bool ResetModelFitter
        {
            get
            {
                return bResetModelFitter;
            }
        }

        public bool Connected
        {
            get
            {
                return bConnected;
            }
        }

        public bool Initialized
        {
            get
            {
                return bInitialized;
            }
        }

        public BackendFaceTracker(int nLandmarks, float confidenceThreshold, int portNumber, LocalFaceTracker localFaceTracker)
        {
            this.nLandmarks = nLandmarks;
            this.confidenceThreshold = confidenceThreshold;
            port = portNumber.ToString();

            this.localFaceTracker = localFaceTracker;
        }

        public async void Initialize()
        {
            listener = new StreamSocketListener();
            listener.ConnectionReceived += Listener_ConnectionReceived;
            await listener.BindServiceNameAsync(port);
            bInitialized = true;
        }

        public void Close()
        {
            bConnected = false;
            bInitialized = false;
            if (listener != null)
                listener.Dispose();
            if (clientSocket != null)
                clientSocket.Dispose();
        }

        public void ModelFitterReset()
        {
            bResetModelFitter = false;
        }

        private async void Listener_ConnectionReceived(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
        {
            clientSocket = args.Socket;
            writer = new DataWriter(clientSocket.OutputStream);
            writer.ByteOrder = Windows.Storage.Streams.ByteOrder.LittleEndian;
            reader = new DataReader(clientSocket.InputStream);

            writer.ByteOrder = Windows.Storage.Streams.ByteOrder.LittleEndian;
            writer.WriteByte(1);
            await writer.StoreAsync();
            await writer.FlushAsync();

            canonicalLandmarks = await ReceiveLandmarks(clientSocket);

            bConnected = true;
        }

        public async Task<float[]> GetLandmarks(SoftwareBitmap image, float[] landmarkInits)
        {
            if (!bConnected)
                return null;

            if (faceTracked)
            {
                if (landmarkInits != null)
                    trackedLandmarks = await TrackLandmarks(image, landmarkInits);
                else
                    trackedLandmarks = await TrackLandmarks(image, trackedLandmarks);

                if (trackedLandmarks != null)              
                    return trackedLandmarks;                
                else
                    faceTracked = false;
            }

            localFaceTracker.ResetTracker();
            float[] landmarks = await localFaceTracker.GetLandmarks(image, landmarkInits);
            if (landmarks == null)
                return null;

            trackedLandmarks = landmarks;
            faceTracked = true;
            bResetModelFitter = true;

            return landmarks;
        }

        private async Task<float[]> TrackLandmarks(SoftwareBitmap image, float[] landmarkInits)
        {
            float[] confidenceAndLandmarks = null;
            try
            {
                await SendLandmarksAndImg(clientSocket, landmarkInits, image);
                confidenceAndLandmarks = await ReceiveConfidenceAndLandmarks(clientSocket);
            }
            catch 
            {
                bConnected = false;
                return null;
            }

            if (confidenceAndLandmarks[0] < confidenceThreshold)
                return null;       

            float[] landmarks = new float[confidenceAndLandmarks.Length - 1];
            System.Buffer.BlockCopy(confidenceAndLandmarks, sizeof(float), landmarks, 0, landmarks.Length * sizeof(float));

            return landmarks;
        }

        private async Task SendLandmarksAndImg(StreamSocket socket, float[] landmarks, SoftwareBitmap image)
        {
            writer.WriteByte(0);

            byte[] byteLandmarks = new byte[landmarks.Length * 4];
            System.Buffer.BlockCopy(landmarks, 0, byteLandmarks, 0, byteLandmarks.Length);
            writer.WriteBuffer(byteLandmarks.AsBuffer());

            byte[] croppedImg = ImageProcessing.PrepareImageForBackend(image, landmarks, canonicalLandmarks, 112, 112);

            writer.WriteInt32(112);
            writer.WriteInt32(112);
            writer.WriteBuffer(croppedImg.AsBuffer());

            await writer.StoreAsync();
            await writer.FlushAsync();
        }

        private async Task<float[]> ReceiveConfidenceAndLandmarks(StreamSocket socket)
        {
            await reader.LoadAsync((uint)nLandmarks * 2 * sizeof(float) + sizeof(float));

            byte[] byteConfidenceAndLandmarks = reader.ReadBuffer((uint)nLandmarks * 2 * sizeof(float) + sizeof(float)).ToArray();
            float[] confidenceAndLandmarks = new float[nLandmarks * 2 + 1];
            System.Buffer.BlockCopy(byteConfidenceAndLandmarks, 0, confidenceAndLandmarks, 0, byteConfidenceAndLandmarks.Length);

            return confidenceAndLandmarks;
        }

        private async Task<float[]> ReceiveLandmarks(StreamSocket socket)
        {
            await reader.LoadAsync((uint)nLandmarks * 2 * sizeof(float));

            byte[] byteLandmarks = reader.ReadBuffer((uint)nLandmarks * 2 * sizeof(float)).ToArray();
            float[] landmarks = new float[nLandmarks * 2];
            System.Buffer.BlockCopy(byteLandmarks, 0, landmarks, 0, byteLandmarks.Length);

            return landmarks;
        }
    }
}
#else

//Empty class provided, so that the project compiles in the editor
namespace FaceProcessing
{
    class BackendFaceTracker
    {       
        public bool ResetModelFitter
        {
            get
            {
                return true;
            }
        }

        public bool Initialized
        {
            get
            {
                return false;
            }
        }

        public bool Connected
        {
            get
            {
                return false;
            }
        }

        public BackendFaceTracker(int nLandmarks, float confidenceThreshold, int portNumber, LocalFaceTracker localFaceTracker)
        {
        }


        public void Initialize()
        {

        }

        public void Close()
        {

        }

        public void ModelFitterReset()
        {

        }

        public Task<float[]> GetLandmarks(byte[] imageData, int height, int width)
        {
            return new Task<float[]>();
        }

    }
}

#endif