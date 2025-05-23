#include <vector>
#include "Debugger/DebuggerUtility.h"
#include "string_view"

namespace sakura {

	//��M�f�[�^�o�b�t�@
	//�ʐM�v���g�R���ɏ]���ăf�[�^���o�b�t�@�����O���؂�o�����s��
	//�����A���ׂẴ��N�G�X�g�𑗂���ςȂ��ł͂Ȃ������ƃ��X�|���X��Ԃ��悤�ɂ���Ȃ�s�v�����ȋC������B
	class ReceivedDataBuffer {
	private:
		std::vector<uint8_t> internalBuffer;

	public:

		//��M�f�[�^�̓���
		void AddReceivedData(const uint8_t* data, size_t size)
		{
			//�����Ƀf�[�^��}��
			size_t currentSize = internalBuffer.size();
			internalBuffer.resize(currentSize + size);
			memcpy(&internalBuffer[currentSize], data, size);
		}

		//��M�f�[�^�̎擾
		bool ReadReceivedString(std::string_view& receivedString)
		{
			//��؂��0�o�C�g������
			void* addr = std::memchr(&internalBuffer[0], 0, internalBuffer.size());
			if (addr != nullptr) {
				//�͈͂�؂�o��
				const size_t size = reinterpret_cast<const uint8_t*>(addr) - &internalBuffer[0];
				receivedString = std::string_view(reinterpret_cast<const char*>(&internalBuffer[0]), size);
				return true;
			}
			else {
				return false;
			}
		}

		//��M�f�[�^�̍폜
		//ReadReceivedString�Ŏ�M���� string_view ��ԋp����`�Ńo�b�t�@���V�t�g���܂�
		void RemoveReceivedData(const std::string_view& receivedString)
		{
			//���ׂĂ̗̈���폜����ꍇ�͓����x�N�^���o�b�t�@�Ƃ��Ďg��
			if (receivedString.size() + 1 == internalBuffer.size()) {
				internalBuffer.clear();
				return;
			}

			//�����炵���̈���Ƃ�Ȃ����ăV�t�g
			std::vector<uint8_t> newBuffer;
			newBuffer.resize(internalBuffer.size() - (receivedString.size() + 1));
			memcpy(&newBuffer[0], &internalBuffer[receivedString.size() + 1], newBuffer.size());
			internalBuffer = std::move(newBuffer);
		}
	};
}