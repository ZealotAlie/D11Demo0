#ifndef _VERTEX_BUFFER_STATE_H_
#define _VERTEX_BUFFER_STATE_H_

template< class TVertex >
class VertexBufferState
{
public:
	VertexBufferState()
		:mOffset(0)
	{}

	void Apply( ID3D11DeviceContext* pContext )
	{
		UINT stride = sizeof(TVertex);
		pContext->IASetInputLayout(TVertex::GetInputLayout());
		pContext->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &mOffset);
		pContext->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void SetOffset( UINT offset )
	{
		mOffset		= offset;
	}

	ID3D11Buffer** GetAddressOfVB()
	{
		return mVB.GetAddressOf();
	}

	ID3D11Buffer** GetAddressOfIB()
	{
		return mIB.GetAddressOf();
	}

private:
	UINT									mOffset;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	mVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	mIB;
};

#endif